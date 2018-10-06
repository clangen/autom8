#include <autom8/autom8.h>

#include <autom8/util/signal_handler.hpp>
#include <autom8/constants.h>

#include <autom8/message/message.hpp>
#include <autom8/message/response.hpp>
#include <autom8/message/request_handler_factory.hpp>
#include <autom8/message/request_handler_registrar.hpp>

#include <autom8/net/server.hpp>
#include <autom8/net/session.hpp>

#include <autom8/util/debug.hpp>
#include <autom8/util/utility.hpp>

#include <autom8/device/null_device_system.hpp>
#include <autom8/device/x10/mochad/mochad_device_system.hpp>

#include <boost/date_time.hpp>
#include <boost/thread/thread.hpp>
#include <sqlite/sqlite3.h>
#include <iostream>

#ifdef WIN32
#include <autom8/device/x10/cm15a/cm15a_device_system.hpp>
#endif

using namespace autom8;

#define REJECT_IF_NOT_INITIALIZED(input) if (!initialized_) { respond_with_status(input, AUTOM8_NOT_INITIALIZED); return; }
#define REJECT_IF_INITIALIZED(input) if (initialized_) { respond_with_status(input, AUTOM8_ALREADY_INITIALIZED); return; }
#define REJECT_IF_SERVER_NOT_STARTED(input) if (server::is_running()) { respond_with_status(input, AUTOM8_SERVER_NOT_RUNNING); return; }
#define REJECT_IF_SERVER_STARTED(input) if (server::is_running()) { respond_with_status(input, AUTOM8_SERVER_ALREADY_RUNNING); return; }

static void no_op(const char*) {
    /* used when rpc callback not specified */
}

/* prototypes, forward decls */
static void process_rpc_request(const std::string& input);
static int system_select(const std::string& system);
static void respond_with_status(json_value_ref input, int status_code);
rpc_callback rpc_callback_ = no_op;

/* constants */
#define VERSION "0.8.1"
#define DEFAULT_PORT 7901
#define TAG "c_api"
#define RPC_TAG "rpc_queue"

/* processing thread */
static volatile int rpc_mode_ = AUTOM8_RPC_MODE_ASYNC;
static boost::thread* io_thread_ = 0;
static boost::recursive_mutex enforce_serial_lock_;
static boost::mutex io_thread_lock_;
static boost::asio::io_service io_service_;

static void io_thread_proc() {
    debug::log(debug::info, RPC_TAG, "thread started");

    /* the io_service will close itself if it thinks there is no
    more work to be done. this line prevents it from auto-stopping */
    boost::asio::io_service::work work(io_service_);
    io_service_.run();

    debug::log(debug::info, RPC_TAG, "thread finished");
}

static void handle_rpc_request(std::string input) {
    try {
        process_rpc_request(input);
    }
    catch (...) {
        debug::log(debug::error, RPC_TAG, "request processing threw for input: " + input);

        try {
            respond_with_status(
                json_value_from_string(input),
                AUTOM8_UNEXPECTED_ERROR
            );
        }
        catch (...) {
            /* handling the error resulted in an... error. give up. */
        }
    }
}

static void enqueue_rpc_request(const std::string& request) {
    boost::mutex::scoped_lock lock(io_thread_lock_);
    io_service_.post(boost::bind(&handle_rpc_request, request));
}

static void start_rpc_queue() {
    boost::mutex::scoped_lock lock(io_thread_lock_);

    if (io_thread_ == 0) {
        io_thread_ = new boost::thread(boost::bind(&io_thread_proc));
    }

    debug::log(debug::info, RPC_TAG, "initialized");
}

static void stop_rpc_queue() {
    boost::mutex::scoped_lock lock(io_thread_lock_);

    io_service_.stop();
    io_thread_->join();
    delete io_thread_;
    io_thread_ = NULL;

    debug::log(debug::info, RPC_TAG, "deinitialized");
}

/* logging */
static log_func external_logger_ = 0;
static boost::mutex external_logger_mutex_;

int autom8_set_logger(log_func logger) {
    boost::mutex::scoped_lock lock(external_logger_mutex_);
    external_logger_ = logger;
    return AUTOM8_OK;
}

int autom8_set_rpc_callback(rpc_callback callback) {
    rpc_callback_ = callback;
    return AUTOM8_OK;
}

class console_logger: public signal_handler {
public:
    console_logger() {
        debug::string_logged.connect(this, &console_logger::on_string_logged);
    }

    void on_string_logged(debug::debug_level level, std::string tag, std::string string) {
        using namespace boost::posix_time;
        using namespace boost::gregorian;
        bool logged = false;

        {
            boost::mutex::scoped_lock lock(external_logger_mutex_);

            if (external_logger_) {
                try {
                    if (external_logger_) {
                        external_logger_((int) level, tag.c_str(), string.c_str());
                        logged = true;
                    }
                }
                catch (...) {
                    std::cerr << "remote log write failed! result will be dumped to stdout." << std::endl;
                }
            }
        }

         /* external process couldn't handle logging. dump to stdout! */
        if (!logged) {
            time_facet* facet(new time_facet("--> [%x %X] "));
            std::cout.imbue(std::locale(std::cout.getloc(), facet));
            ptime time(microsec_clock::local_time());
            std::cout << time << string << std::endl;
        }
    }
};

static console_logger* default_logger_ = 0;

/* init, deinit */
static bool initialized_ = false;

int autom8_init(int rpc_mode) {
    if (initialized_) {
        return AUTOM8_ALREADY_INITIALIZED;
    }

    sqlite3_config(SQLITE_CONFIG_SERIALIZED);

    rpc_mode_ = rpc_mode;
    if (rpc_mode == AUTOM8_RPC_MODE_ASYNC) {
        start_rpc_queue();
    }

    {
        boost::mutex::scoped_lock lock(external_logger_mutex_);
        default_logger_ = new console_logger();
        debug::init();
    }

    /* select the last selected system, or null by default */
    std::string system = "null";
    utility::prefs().get("system.selected", system);
    system_select(system);

    initialized_ = true;

    return AUTOM8_OK;
}

int autom8_deinit() {
    if (!initialized_) {
        return AUTOM8_NOT_INITIALIZED;
    }

    server::stop();

    {
        boost::mutex::scoped_lock lock(external_logger_mutex_);

        debug::deinit();

        if (default_logger_) {
            delete default_logger_;
            default_logger_ = 0;
        }
    }

    external_logger_ = 0;
    initialized_ = false;

    stop_rpc_queue();

    return AUTOM8_OK;
}

/* util */
static void respond_with_status(json_value_ref input, int status_code) {
    json_value_ref response = json_value_ref(new json_value());
    (*response)["id"] = (*input)["id"].asString();
    (*response)["status"] = status_code;
    (*response)["message"] = json_value(Json::objectValue);
    rpc_callback_(json_value_to_string(*response).c_str());
}

static void respond_with_status(json_value_ref input, const std::string& errmsg) {
    json_value_ref response = json_value_ref(new json_value());
    (*response)["id"] = (*input)["id"].asString();
    (*response)["status"] = AUTOM8_UNKNOWN;
    (*response)["message"] = errmsg;
    rpc_callback_(json_value_to_string(*response).c_str());
}

static void respond_with_status(json_value_ref input, json_value_ref json) {
    /* note if json looks like this: {status: ..., message: ...} the status
    code will be automatically extracted. otherwise AUTOM8_OK will be used */

    json_value_ref response = json_value_ref(new json_value());

    (*response)["id"] = (*input)["id"].asString();

    int status = (int) json->get("status", AUTOM8_OK).asInt();
    (*response)["status"] = status;

    json_value message = json->get("message", json_value(Json::nullValue));
    if (!message.isNull()) {
        (*response)["message"] = json->get("message", "");
    }
    else {
        (*response)["message"] = *json;
    }

    rpc_callback_(json_value_to_string(*response).c_str());
}

/* server command handlers */
static int server_start();
static int server_stop();
static int server_set_preference(json_value& options);
static json_value_ref server_get_preference(json_value& options);
static json_value_ref server_status();

static void handle_server(json_value_ref input) {
    std::string command = input->get("command", "").asString();
    json_value options = input->get("options", json_value());

    if (options.type() != Json::objectValue) {
        options = json_value(Json::objectValue);
    }

    if (command == "status") {
        respond_with_status(input, server_status());
    }
    else if (command == "start") {
        respond_with_status(input, server_start());
    }
    else if (command == "stop") {
        respond_with_status(input, server_stop());
    }
    else if (command == "set_preference") {
        REJECT_IF_SERVER_STARTED(input)
        respond_with_status(input, server_set_preference(options));
    }
    else if (command == "get_preference") {
        REJECT_IF_SERVER_STARTED(input)
        respond_with_status(input, server_get_preference(options));
    }
    else {
        respond_with_status(input, AUTOM8_INVALID_COMMAND);
    }
}

int server_set_preference(json_value& options) {
    std::string key = options.get("key", "").asString();
    std::string value = options.get("value", "").asString();

    if (key.size() == 0 || value.size() == 0) {
        return AUTOM8_INVALID_ARGUMENT;
    }

    return (utility::prefs().set(key, value) ? AUTOM8_OK : AUTOM8_UNKNOWN);
}

json_value_ref server_get_preference(json_value& options) {
    json_value_ref result(new json_value());
    (*result)["status"] = AUTOM8_OK;

    std::string key = options.get("key", "").asString();
    if (key.size() == 0) {
        (*result)["status"] = AUTOM8_INVALID_ARGUMENT;
        (*result)["message"] = "key not specified";
    }
    else {
        std::string value = "__INVALID__";
        utility::prefs().get(key, value);

        if (value == "__INVALID__") {
            (*result)["status"] = AUTOM8_INVALID_ARGUMENT;
            (*result)["message"] = "key not found";
        }
        else {
            json_value message;
            message["key"] = key;
            message["value"] = value;
            (*result)["message"] = message;
        }
    }

    return result;
}

json_value_ref server_status() {
    json_value_ref result(new json_value());

    std::string system = "null", fingerprint = "unknown", port = "7901", webClientPort = "7902";
    utility::prefs().get("system.selected", system);
    utility::prefs().get("fingerprint", fingerprint);
    utility::prefs().get("port", port);
    utility::prefs().get("webClientPort", webClientPort);

    std::string description;
    if (device_system::instance()) {
        description = device_system::instance()->description();
    }

    (*result)["system_id"] = system;
    (*result)["system_description"] = description;
    (*result)["fingerprint"] = fingerprint;
    (*result)["running"] = server::is_running();
    (*result)["version"] = std::string(VERSION);
    (*result)["port"] = port;
    (*result)["webClientPort"] = webClientPort;

    return result;
}

int server_start() {
    if (server::is_running() && !server_stop()) {
        return -999; /* can't stop server is fatal */
    }

    int port = DEFAULT_PORT;
    utility::prefs().get("port", port);

    server::start(port);

    return 1;
}

int server_stop() {
    if (server::stop()) {
        return AUTOM8_OK;
    }

    return AUTOM8_FALSE;
}

/* system command handlers */
static json_value_ref system_list() {
    json_value list = json_value(Json::arrayValue);

#if WIN32
    list.append("cm15a");
#endif
    list.append("mochad");
    list.append("null/mock");

    json_value_ref result(new json_value());
    (*result)["systems"] = list;
    return result;
}

static json_value_ref system_selected() {
    json_value_ref result(new json_value());

    std::string system = "null";
    utility::prefs().get("system.selected", system);
    (*result)["system_id"] = system;

    return result;
}

static int system_select(json_value& options) {
    return system_select(options["system"].asString());
}

static int system_select(const std::string& system) {
    if (system == "null" || system == "null/mock") {
        device_system::set_instance(
            device_system_ptr(new null_device_system())
        );
    }
    else if (system == "mochad") {
        device_system::set_instance(
            device_system_ptr(new mochad_device_system())
        );
    }
#ifdef WIN32
    else if (system == "cm15a") {
        device_system::set_instance(
            device_system_ptr(new cm15a_device_system())
        );
    }
#endif
    else {
        return AUTOM8_INVALID_SYSTEM;
    }

    utility::prefs().set("system.selected", system);
    return AUTOM8_OK;
}

static json_value_ref system_list_devices() {
    device_system& ds = *device_system::instance();
    device_model& model = ds.model();
    device_list devices;
    model.all_devices(devices);

    json_value list = json_value(Json::arrayValue);
    for (size_t i = 0; i < devices.size(); i++) {
        list.append(*(devices.at(i)->to_json()));
    }

    json_value_ref result(new json_value());
    (*result)["devices"] = list;
    return result;
}

void json_array_to_vector(const json_value& value, std::vector<std::string>& target) {
    if (value.isArray()) {
        for (unsigned i = 0; i < value.size(); i++) {
            target.push_back(value.get(i, "").asString());
        }
    }
}

static json_value_ref system_edit_device(json_value& options) {
    json_value_ref result(new json_value()); /* output */

    std::string original_address = options.get("address", "").asString();
    device_model& model = device_system::instance()->model();
    device_ptr device = model.find_by_address(original_address);

    if (device) {
        json_value new_values = options.get("device", "");

        std::string address = new_values.get("address", "").asString();
        std::string label = new_values.get("label", "").asString();
        device_type type = (device_type) new_values.get("type", (int) device_type_unknown).asInt();

        /* json array -> std::vector<> */
        json_value groups_json = new_values.get("groups", "");
        std::vector<std::string> groups;
        json_array_to_vector(groups_json, groups);

        /* we should have everything we need to update now... */
        if (address.size() > 0 && label.size() > 0) {
            std::transform(address.begin(), address.end(), address.begin(), ::tolower);

            bool updated = model.update(
                device->id(), type, address, label, groups
            );

            if (updated) {
                device = model.find_by_address(address);
                (*result)["device"] = *(device->to_json());
                return result;
            }
        }

        (*result)["message"] = "parameters specified, but invalid " + original_address;
        (*result)["status"] = AUTOM8_DEVICE_NOT_FOUND;
    }
    else {
        (*result)["message"] = "device update failed: could not find device " + original_address;
        (*result)["status"] = AUTOM8_DEVICE_NOT_FOUND;
    }

    return result;
}

static json_value_ref system_add_device(json_value& options) {
    std::string address = options.get("address", "").asString();
    std::string label = options.get("label", "").asString();
    device_type type = (device_type) options.get("type", (int) device_type_unknown).asInt();

    json_value_ref result(new json_value()); /* output */

    if (address.size() == 0 || label.size() == 0 || type == device_type_unknown) {
        (*result)["message"] = "address, label, or type invalid";
        (*result)["status"] = AUTOM8_INVALID_ARGUMENT;
    }
    else {
        /* json array -> std::vector<> */
        json_value groups_json = options.get("groups", ""); /* read */
        std::vector<std::string> groups;
        json_array_to_vector(groups_json, groups);

        device_model& model = device_system::instance()->model();
        device_ptr device = model.find_by_address(address);

        if (device) {
            json_value device_json; /* ugh, so verbose */
            device_json["device"] = *device->to_json();

            (*result)["message"] = (*result)["message"] = device_json;
            (*result)["status"] = AUTOM8_DEVICE_ALREADY_EXISTS;
        }
        else {
            device = model.add(type, address, label, groups);

            if (device) {
                (*result)["device"] = *(device->to_json());
            }
            else {
                (*result)["message"] = "failed to create device";
                (*result)["status"] = AUTOM8_INVALID_ARGUMENT;
            }
        }
    }

    return result;
}

static int system_delete_device(json_value& options) {
    std::string address = options.get("address", "").asString();

    if (address.length() == 0) {
        return AUTOM8_INVALID_ARGUMENT;
    }

    device_model& model = device_system::instance()->model();
    device_ptr device = model.find_by_address(address);

    if (!device) {
        return AUTOM8_DEVICE_NOT_FOUND;
    }

    if (!model.remove(device)) {
        return AUTOM8_UNKNOWN;
    }

    return AUTOM8_OK;
}

static void handle_system(json_value_ref input) {
    std::string command = input->get("command", "").asString();
    json_value options = input->get("options", json_value());

    if (options.type() != Json::objectValue) {
        options = json_value(Json::objectValue);
    }

    if (command == "list") {
        respond_with_status(input, system_list());
    }
    else if (command == "selected") {
        respond_with_status(input, system_selected());
    }
    else if (command == "list_devices") {
        respond_with_status(input, system_list_devices());
    }
    else {
        REJECT_IF_SERVER_STARTED(input)
        else if (command == "select") {
            respond_with_status(input, system_select(options));
        }
        else if (command == "add_device") {
            respond_with_status(input, system_add_device(options));
        }
        else if (command == "edit_device") {
            respond_with_status(input, system_edit_device(options));
        }
        else if (command == "delete_device") {
            respond_with_status(input, system_delete_device(options));
        }
        else {
            respond_with_status(input, AUTOM8_INVALID_COMMAND);
        }
    }
}

/* generic rpc interface */
void autom8_rpc(const char* input) {
    if (rpc_mode_ == AUTOM8_RPC_MODE_ASYNC) {
        enqueue_rpc_request(std::string(input));
    }
    else {
        /* the enqueue above will guarantee serial processing of requests.
        in sync mode we need to take extra care to do this ourselves.
        NOTE: we use a recursive_lock here just in case a poorly implemented
        client decides to make another RPC call while processing the result
        of another call, leading to a re-entrant autom8_rpc() call. */
        boost::recursive_mutex::scoped_lock lock(enforce_serial_lock_);
        handle_rpc_request(std::string(input));
    }
}

static void process_rpc_request(const std::string& input) {
    json_value_ref parsed;

    try {
        parsed = json_value_from_string(input);
    }
    catch (...) {
        /* we will return in a sec */
    }

    REJECT_IF_NOT_INITIALIZED(parsed)

    if (!parsed) {
        debug::log(debug::error, TAG, "autom8_rpc input parse failed");
        respond_with_status(parsed, AUTOM8_PARSE_ERROR);
        return;
    }

    const std::string component = parsed->get("component", "").asString();
    const std::string command = parsed->get("command", "").asString();

    debug::log(debug::info, TAG, (std::string("handling '") + component + "' command '" + command + "'"));

    if (component == "server") {
        handle_server(parsed);
    }
    else if (component == "system") {
        handle_system(parsed);
    }
    else {
        debug::log(debug::error, TAG, std::string("invalid component '") + component + "' specified. rpc call ignored");
    }
}

const char* autom8_version() {
    return VERSION;
}
