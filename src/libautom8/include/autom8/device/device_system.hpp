#pragma once

#include <memory>
#include <autom8/device/device_base.hpp>
#include <autom8/device/device_model.hpp>

namespace autom8 {
    class device_system;
    typedef std::shared_ptr<device_system> device_system_ptr;

    class device_system {
    public:
        virtual std::string description() = 0;
        virtual device_model& model() = 0;

        static device_system_ptr instance();
        static device_system_ptr set_instance(const std::string& type);
        static std::vector<std::string> types();
        static std::string default_type();
        static void clear_instance();

    protected:
        device_system() { }
        virtual ~device_system() { }
    };
}
