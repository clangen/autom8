#pragma once

#include <autom8/device/device_system.hpp>

namespace autom8 {
    class x10_device_system : public device_system {
    public:
        virtual bool send_device_message(command_type message_type, const char* message_params) = 0;
        virtual void requery_device_status(const std::string& address) = 0;
        virtual std::string controller_type() const = 0;
    };
}
