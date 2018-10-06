#ifndef __C_AUTOM8_COMMON_MESSAGES_HPP__
#define __C_AUTOM8_COMMON_MESSAGES_HPP__

#include <autom8/message/response.hpp>
#include <autom8/message/request.hpp>
#include <autom8/device/device_base.hpp>

namespace autom8 {
    namespace messages {
        namespace requests {
            request_ptr ping();
            request_ptr authenticate(const std::string& pw);
            request_ptr get_device_list();
        }  // requests

        namespace responses {
            response_ptr authenticated();
            response_ptr authenticate_failed();

            response_ptr device_status_updated(device_ptr);
            response_ptr device_status_updating(device_ptr);
            response_ptr sensor_status_changed(device_ptr);

            response_ptr pong();
        } // responses
    } // messages
} // autom8

#endif