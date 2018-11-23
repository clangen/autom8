#pragma once

#include <memory>
#include <autom8/device/device_base.hpp>
#include <autom8/device/device_model.hpp>
#include <f8n/sdk/ISchema.h>

namespace autom8 {
    class device_system;
    using device_system_ptr = std::shared_ptr<device_system>;
    using schema_ptr = std::shared_ptr<f8n::sdk::ISchema>;

    class device_system {
    public:
        virtual std::string description() = 0;
        virtual device_model& model() = 0;
        virtual schema_ptr schema() = 0;
        virtual void on_settings_changed() = 0;

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
