#pragma once

#include <autom8/util/signal_handler.hpp>
#include <autom8/device/device_system.hpp>

namespace autom8 {
    class null_device_system: public device_system, public signal_handler {
    public:
        null_device_system();
        virtual std::string description() { return "null/mock"; }
        virtual device_model& model() { return model_; }

    private:
        void on_device_removed(database_id id);
        void on_device_updated(database_id id);


    private:
        class null_device_factory: public device_factory {
        public:
            virtual device_ptr create(
                database_id id,
                device_type type,
                const std::string& address,
                const std::string& label,
                const std::vector<std::string>& groups
            );

            virtual std::string name() const;
        };

    private:
        device_model model_;
    };
}
