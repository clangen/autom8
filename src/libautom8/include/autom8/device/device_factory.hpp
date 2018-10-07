#pragma once

#include <memory>
#include <autom8/device/device_base.hpp>

namespace autom8 {
    class device_system;
    typedef std::shared_ptr<device_system> device_system_ptr;

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

    typedef std::shared_ptr<device_factory> device_factory_ptr;
}
