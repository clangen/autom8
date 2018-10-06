#ifndef __C_AUTOM8_X10_APPLIANCE_HPP__
#define __C_AUTOM8_X10_APPLIANCE_HPP__

#include <autom8/device/x10/x10_device.hpp>

namespace autom8 {
    class x10_appliance : public x10_device {
    public:
        x10_appliance(
            x10_device_system* owner,
            database_id id,
            const std::string& x10_address,
            const std::string& label);

        virtual ~x10_appliance();
        virtual device_type type();
    };
}

#endif