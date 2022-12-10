#include <autom8/net/server.hpp>
#include <autom8/message/response.hpp>
#include <autom8/device/device_system.hpp>
#include <autom8/message/requests/get_device_list.hpp>

#include <json.hpp>

using namespace nlohmann;
using namespace autom8;

//////////////

class get_device_list_response: public response {
public:
    virtual std::string uri() {
        return "autom8://response/get_device_list";
    }

    virtual std::shared_ptr<json> body() {
        device_list devices = device_system::instance()->model().all_devices();

        auto body = std::make_shared<json>();
        auto& devices_node = (*body)["devices"] = json::array();

        for (auto device : devices) {
            devices_node.push_back(device->to_json());
        }

        return body;
    }

    response::response_target target() {
        return response::requester_only;
    }
};

//////////////

get_device_list::get_device_list() {
}

bool get_device_list::can_handle(session_ptr session, message_ptr request) {
    return (request->name() == "get_device_list");
}

void get_device_list::operator()(session_ptr session, message_ptr request) {
    server::send(session, response_ptr(new get_device_list_response()));
}

request_handler_ptr get_device_list::create() {
    return request_handler_ptr(new get_device_list());
}

request_ptr get_device_list::request() {
    return request::create("autom8://request/get_device_list");
}

//////////////

