#ifndef __C_AUTOM8_MOCHAD_DEVICE_SYSTEM_HPP__
#define __C_AUTOM8_MOCHAD_DEVICE_SYSTEM_HPP__

#include <boost/shared_ptr.hpp>

#include <autom8/util/signal_handler.hpp>
#include <autom8/device/x10/x10_device_system.hpp>
#include <autom8/device/x10/x10_device.hpp>
#include <autom8/device/device_model.hpp>
#include <autom8/device/x10/mochad/mochad_controller.hpp>

namespace autom8 {
    class mochad_device_system: public x10_device_system, public signal_handler {
    public:
        mochad_device_system();
        virtual ~mochad_device_system();

        virtual std::string description() { return "mochad (cm11a/cm15a/cm19)"; }
        virtual device_model& model();
        virtual bool send_device_message(command_type message_type, const char* message_params);
        virtual std::string controller_type() const { return "mochad"; }
        virtual void requery_device_status(const std::string& address);

        virtual void on_message_received(std::string);

    private:
        void on_device_removed(database_id id);
        void on_device_updated(database_id id);

        device_list devices_;
        mochad_controller controller_;
        device_model_ptr model_;
        device_factory_ptr factory_;
        std::string last_house_unit_;
    };
}

#endif