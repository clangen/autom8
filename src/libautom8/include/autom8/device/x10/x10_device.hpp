#ifndef __C_AUTOM8_X10_DEVICE_HPP__
#define __C_AUTOM8_X10_DEVICE_HPP__

#include <boost/shared_ptr.hpp>
#include <autom8/device/simple_device.hpp>
#include <autom8/device/x10/x10_device_system.hpp>

namespace autom8 {
    class x10_device_system;

    class x10_device: public simple_device {
    public:
        x10_device(
            x10_device_system* owner,
            database_id id,
            const std::string& address,
            const std::string& label,
            const std::vector<std::string>& groups = std::vector<std::string>());

        virtual ~x10_device();

        virtual void on_controller_message(const std::vector<std::string>& status_values);
        virtual void turn_on();
        virtual void turn_off();

        virtual device_type type() = 0; /* implemented by concrete classes */

    protected:
        x10_device_system* owner() { return owner_; }
        virtual void set_device_status(device_status new_status);

    private:
        x10_device_system* owner_;
    };
}

#endif