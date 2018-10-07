#pragma once

#include <autom8/device/x10/x10_device.hpp>

namespace autom8 {
    class x10_security_sensor: public x10_device, public security_sensor {
    private:
        typedef x10_device base;
        friend class reset_sensor_status;

    public:
        x10_security_sensor(
            x10_device_system* owner,
            database_id id,
            const std::string& x10_address,
            const std::string& label);

        virtual ~x10_security_sensor();

        virtual device_type type();
        virtual device_status status();

        virtual void arm();
        virtual void disarm();
        virtual void reset();
        virtual bool is_armed() { return is_armed_; }
        virtual bool is_tripped() { return is_tripped_; }

        virtual void on_controller_message(
            const std::vector<std::string>& status_values);

    protected:
        virtual void on_status_changed();
        virtual void get_extended_json_attributes(nlohmann::json& target);

    protected:
        bool is_tripped_;
        bool is_armed_;
    };
}
