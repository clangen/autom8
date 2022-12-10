#include <vector>
#include <string>
#include <iostream>

#include <f8n/str/util.h>

#include <autom8/message/common_messages.hpp>
#include <autom8/net/server.hpp>
#include <autom8/device/x10/x10_device.hpp>
#include <autom8/device/x10/x10_device_system.hpp>

using namespace autom8;

x10_device::x10_device(
    x10_device_system* owner,
    database_id id,
    const std::string& address,
    const std::string& label,
	const std::vector<std::string>& groups)
: simple_device(id, address, label, groups)
, owner_(owner) {
    owner_->requery_device_status(address);
}

x10_device::~x10_device() {
}

void x10_device::turn_on() {
    set_device_status(device_status_on);
}

void x10_device::turn_off() {
    set_device_status(device_status_off);
}

void x10_device::set_device_status(device_status new_status) {
    server::send(messages::responses::device_status_updating(shared_from_this()));

    std::string command_string;

    {
        auto lock = state_lock();

        command_string = f8n::str::format(
            "%s %s",
            address().c_str(),
            (new_status == device_status_off ? "off" : "on"));
    }

    owner_->send_device_message(powerline_command, command_string.c_str());
}

void x10_device::on_controller_message(const std::vector<std::string>& status_values) {
    std::string type(status_values[2]);
    std::transform(type.begin(), type.end(), type.begin(), tolower);

    // device status
    if ((type == "off") || (type == "on")) {
        {
            auto lock = state_lock();
            status_ = (type == "off") ? device_status_off : device_status_on;
        }

        on_status_changed();
        server::send(messages::responses::device_status_updated(shared_from_this()));
    }
}
