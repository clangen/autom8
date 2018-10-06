#ifndef __C_AUTOM8_DEVICE_FACTORY_HPP__
#define __C_AUTOM8_DEVICE_FACTORY_HPP__

#include <boost/shared_ptr.hpp>
#include <autom8/device/device_base.hpp>

namespace autom8 {
    class device_system;
    typedef boost::shared_ptr<device_system> device_system_ptr;

    class device_factory {
    public:
        virtual device_ptr create(
            database_id id,
            device_type type,
            const std::string& address,
            const std::string& label,
            const std::vector<std::string>& groups) = 0;

        virtual std::string name() const = 0;
    };

    typedef boost::shared_ptr<device_factory> device_factory_ptr;
}

#endif