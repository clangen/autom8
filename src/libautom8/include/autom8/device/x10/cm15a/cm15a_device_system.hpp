#ifndef __C_AUTOM8_CM15A_DEVICE_SYSTEM_HPP__
#define __C_AUTOM8_CM15A_DEVICE_SYSTEM_HPP__

#include <windows.h>
#include <boost/shared_ptr.hpp>
#include <autom8/util/signal_handler.hpp>
#include <autom8/device/device_model.hpp>
#include <autom8/device/x10/x10_device_system.hpp>
#include <autom8/device/x10/x10_device.hpp>
#include <autom8/device/x10/x10_device_controller.hpp>

namespace autom8 {
    class cm15a_device_system: public x10_device_system, public signal_handler {
    public:
        cm15a_device_system();
        virtual ~cm15a_device_system();

        virtual std::string description() { return "cm15a (usb)"; }
        virtual device_model& model();
        virtual bool send_device_message(command_type message_type, const char* message_params);
        virtual std::string controller_type() const { return "cm15a"; }
        virtual void requery_device_status(const std::string& address);

        virtual void on_message_received(const char ** argv, int argc);

    private:
        void on_device_removed(database_id id);
        void on_device_updated(database_id id);

        HMODULE dll_;
        device_list devices_;
        x10_device_controller controller_;
        device_model_ptr model_;
        device_factory_ptr factory_;
        bool is_functional_;
    };
}

#endif // __C_AUTOM8_CM15A_DEVICE_SYSTEM_HPP__