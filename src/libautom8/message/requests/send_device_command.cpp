#include <autom8/device/device_system.hpp>
#include <autom8/message/response.hpp>
#include <autom8/net/server.hpp>
#include <autom8/util/debug.hpp>
#include <autom8/message/requests/send_device_command.hpp>

#include <vector>
#include <json.hpp>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

using namespace autom8;

static std::string set_status_command = "set_status";
static std::string set_brightness_command = "set_brightness";
static std::string reset_sensor_status_command = "reset_sensor_status";
static std::string arm_sensor_command = "arm_sensor";

static std::string TAG = "send_device_command";

using namespace nlohmann;

static inline std::string to_string(const json& j) {
    if (j.type() == json::value_t::string) {
        return j.get<std::string>();
    }
    return j.dump();
}

send_device_command::send_device_command() {

}

request_handler_ptr send_device_command::create() {
    return request_handler_ptr(new send_device_command());
}

bool send_device_command::can_handle(session_ptr, message_ptr request) {
    return (request->name() == "send_device_command");
}

void send_device_command::operator()(session_ptr session, message_ptr message) {
    std::string address, name;
    int type = -1;
    param_list params;

    try {
        const json& document = message->body();
        const json& command = document["command"];
        const json& parameters = command["parameters"];

        name = command.value("name", "");
        type = command.value("type", -1);
        address = command.value("address", "");

        typedef std::vector<std::string> key_list;
        typedef key_list::iterator iterator;

        for (auto it = parameters.begin(); it != parameters.end(); ++it) {
            params[it.key()] = to_string(it.value());
        }
    }
    catch (...) {
        debug::log(debug::warning, TAG, "set_device_status: message body parse failed!");
        return;
    }

    device_ptr device = device_system::instance()->model().find_by_address(address);
    if (device) {
        try {
            dispatch(device, name, params);
        }
        catch (...) {
            debug::log(debug::warning, TAG, "failed to dispatch send_device_command message! invalid message!");
        }
    }
}

void send_device_command::dispatch(
    device_ptr device,
    const std::string& command,
    const param_list& params)
{
    typedef param_list::const_iterator iterator;

    if (command == set_status_command) {
        iterator it = params.find("status");
        if (it != params.end()) {
            device_status new_status = (device_status) boost::lexical_cast<int>(it->second);
            (new_status == device_status_on) ? device->turn_on() : device->turn_off();
        }
    }
    else if (command == set_brightness_command) {
        iterator it = params.find("brightness");
        if (it != params.end()) {
            int brightness = boost::lexical_cast<int>(it->second);

            lamp* l = NULL;
            if (device->get_interface(l)) {
                l->set_brightness(brightness);
            }
        }
    }
    else if (command == reset_sensor_status_command) {
        security_sensor* sensor;
        if (device->get_interface(sensor)) {
            sensor->reset();
        }
    }
    else if (command == arm_sensor_command) {
        iterator it = params.find("set_armed");
        if (it != params.end()) {
            bool armed = (it->second == "true");

            security_sensor* sensor;
            if (device->get_interface(sensor)) {
                armed ? sensor->arm() : sensor->disarm();
            }
        }
    }
}