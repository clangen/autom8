#include <autom8/message/common_messages.hpp>

using namespace autom8;
using namespace nlohmann;

typedef response::response_target response_target;

namespace autom8 {
    namespace messages {
        namespace requests {
            request_ptr ping() {
                return request::create("autom8://request/ping");
            }

            request_ptr authenticate(const std::string& pw) {
                json body;
                body["password"] = pw;

                return request::create("autom8://request/authenticate", body);
            }

            request_ptr get_device_list() {
                return request::create("autom8://request/get_device_list");
            }
        } // requests

        namespace responses {
            response_ptr authenticated() {
                return response::create("autom8://response/authenticated");
            }

            response_ptr authenticate_failed() {
                return response::create("autom8://response/authenticate_failed");
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
                return response::create("autom8://response/pong");
            }
        } // responses
    } // messages
} // autom8
