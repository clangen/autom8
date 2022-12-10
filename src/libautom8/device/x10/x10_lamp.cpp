#include <vector>
#include <string>
#include <iostream>

#include <f8n/str/util.h>

#include <autom8/message/common_messages.hpp>
#include <autom8/net/server.hpp>
#include <autom8/device/x10/x10_lamp.hpp>
#include <autom8/device/x10/x10_device_system.hpp>

using namespace autom8;

x10_lamp::x10_lamp(
    x10_device_system* owner,
    database_id id,
    const std::string& x10_address,
    const std::string& label)
: x10_device(owner, id, x10_address, label)
, brightness_(100)
, last_brightness_(100) {
}

x10_lamp::~x10_lamp() {
}

device_type x10_lamp::type() {
    return device_type_lamp;
}

void x10_lamp::on_controller_message(const std::vector<std::string>& status_values) {
    std::string type(status_values[2]);
    std::transform(type.begin(), type.end(), type.begin(), tolower);

    {
        auto lock = state_lock();

        // X10 isn't smart enough to restore brightness state when we
        // turn a lamp off, then back on. first, remember the current brightness...
        if (type == "off") {
            last_brightness_ = brightness_;
            brightness_ = 100;
        }
    }

    base::on_controller_message(status_values);

    // ... now, when the device is turned back on, restore the brightness.
    if (type == "on") {
        set_brightness(last_brightness_);
    }
}

void x10_lamp::get_extended_json_attributes(nlohmann::json& target) {
    auto lock = state_lock();
    target["brightness"] = brightness_;
}

void x10_lamp::set_brightness(int new_brightness) {
    std::string command_string;
    bool update = false;

    {
        auto lock = state_lock();

        if (status_ == device_status_off) {
            last_brightness_ = new_brightness;
            return;
        }

        // when you adjust the brightness you first need to determine whether the
        // new value will cause the lamp to bright, or dim, then calculate a delta.
        if (new_brightness != brightness_) {
            std::string command = (new_brightness > brightness_) ? "bright" : "dim";
            int delta = abs(new_brightness - brightness_);

            // build the string
            command_string = f8n::str::format(
                "%s %s %d", address_.c_str(), command.c_str(), delta);

            // update clients
            brightness_ = new_brightness;
            update = true;
        }
    }

    if (update) {
        owner()->send_device_message(powerline_command, command_string.c_str());
        on_status_changed();
        server::send(messages::responses::device_status_updated(shared_from_this()));
    }
}

int x10_lamp::brightness() {
    return (status() == device_status_off) ? last_brightness_ : brightness_;
}
