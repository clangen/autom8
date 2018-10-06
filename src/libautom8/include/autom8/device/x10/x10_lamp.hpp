#ifndef __C_AUTOM8_X10_LAMP_HPP__
#define __C_AUTOM8_X10_LAMP_HPP__

#include <autom8/device/x10/x10_device.hpp>

namespace autom8 {
    class x10_lamp: public x10_device, public lamp {
    private:
        typedef x10_device base;

    public:
        x10_lamp(
            x10_device_system* owner,
            database_id id,
            const std::string& x10_address,
            const std::string& label);

        virtual ~x10_lamp();

        virtual device_type type();
        virtual int brightness();
        virtual void set_brightness(int brightness);
        virtual void on_controller_message(const std::vector<std::string>& status_values);

    protected:
        virtual void get_extended_json_attributes(json_value& target);

    private:
        int brightness_;
        int last_brightness_;
    };
}

#endif