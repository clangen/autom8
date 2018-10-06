#include <boost/format.hpp>

#include <autom8/net/server.hpp>
#include <autom8/message/response.hpp>
#include <autom8/device/device_system.hpp>
#include <autom8/message/requests/get_device_list.hpp>

#include <json/writer.h>

using namespace autom8;

//////////////

class get_device_list_response: public response {
public:
    virtual std::string uri() {
        return "autom8://response/get_device_list";
    }

    virtual json_value_ref body() {
        device_list devices;
        device_system::instance()->model().all_devices(devices);

        json_value_ref body(new json_value());
        json_value& devices_node = (*body)["devices"] = Json::Value(Json::arrayValue);

        for (size_t i = 0; i < devices.size(); i++) {
            devices_node.append((*devices[i]->to_json()));
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

//////////////

