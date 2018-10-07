#include <boost/format.hpp>

#include <autom8/net/server.hpp>
#include <autom8/message/response.hpp>
#include <autom8/device/device_system.hpp>
#include <autom8/message/requests/get_security_alert_count.hpp>

#include <json.hpp>

using namespace autom8;
using namespace nlohmann;

bool get_security_alert_count::can_handle(session_ptr session, message_ptr request) {
    return (request->name() == "get_security_alert_count");
}

void get_security_alert_count::operator()(session_ptr session, message_ptr request) {
    device_list devices;
    if (device_system::instance()->model().all_devices(devices)) {
        int count = 0;

        // TODO: add device_model::find_by_type() or something.
        device_list::iterator it = devices.begin();
        for ( ; it != devices.end(); it++) {
            device_ptr device = *it;
            if (device->type() == device_type_security_sensor) {
                security_sensor* sensor;
                if (device->get_interface(sensor)) {
                    if (sensor->is_tripped() && sensor->is_armed()) {
                        ++count;
                    }
                }
            }
        }

        // build JSON body
        nlohmann::json document;
        document["security_alert_count"] = count;

        // send response
        response_ptr response = response::create(
            "autom8://response/get_security_alert_count",
            document,
            response::requester_only);

        server::send(session, response);
    }
}

request_handler_ptr get_security_alert_count::create() {
    return request_handler_ptr(new get_security_alert_count());
}

