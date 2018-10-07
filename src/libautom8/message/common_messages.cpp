#include <autom8/message/common_messages.hpp>

using namespace autom8;
using namespace nlohmann;

typedef response::response_target response_target;

static const json blank_body_;

namespace autom8 {
    namespace messages {
        namespace requests {
            request_ptr ping() {
                return request::create("autom8://request/ping", blank_body_);
            }

            request_ptr authenticate(const std::string& pw) {
                json body;
                body["password"] = pw;

                return request::create("autom8://request/authenticate", body);
            }

            request_ptr get_device_list() {
                return request::create("autom8://request/get_device_list", blank_body_);
            }
        } // requests

        namespace responses {
            response_ptr authenticated() {
                return response::create("autom8://response/authenticated", blank_body_);
            }

            response_ptr authenticate_failed() {
                return response::create("autom8://response/authenticate_failed", blank_body_);
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
                return response::create("autom8://response/pong", blank_body_);
            }
        } // responses
    } // messages
} // autom8
