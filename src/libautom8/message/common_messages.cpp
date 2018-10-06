#include <autom8/message/common_messages.hpp>

using namespace autom8;
typedef response::response_target response_target;

static json_value_ref blank_body() {
    return json_value_ref(new json_value(json::objectValue));
}

namespace autom8 {
    namespace messages {
        namespace requests {
            request_ptr ping() {
                return request::create("autom8://request/ping", blank_body());
            }

            request_ptr authenticate(const std::string& pw) {
                json_value_ref body = json_value_ref(new json_value());
                (*body)["password"] = pw;

                return request::create("autom8://request/authenticate", body);
            }

            request_ptr get_device_list() {
                return request::create("autom8://request/get_device_list", blank_body());
            }
        } // requests

        namespace responses {
            response_ptr authenticated() {
                return response::create("autom8://response/authenticated", blank_body());
            }

            response_ptr authenticate_failed() {
                return response::create("autom8://response/authenticate_failed", blank_body());
            }

            response_ptr device_status_updated(device_ptr d) {
                return response::create(
                    "autom8://response/device_status_updated",
                    d->to_json(),
                    response::all_sessions);
            }

            response_ptr device_status_updating(device_ptr d) {
                return response::create(
                    "autom8://response/device_status_updating",
                    d->to_json(),
                    response::all_sessions);
            }

            response_ptr sensor_status_changed(device_ptr d) {
                return response::create(
                    "autom8://response/sensor_status_changed",
                    d->to_json(),
                    response::all_sessions);
            }

            response_ptr pong() {
                return response::create("autom8://response/pong", blank_body());
            }
        } // responses
    } // messages
} // autom8
