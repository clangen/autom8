#ifndef __C_AUTOM8_RESPONSE_HANDLER_SEND_DEVICE_COMMAND_HPP_
#define __C_AUTOM8_RESPONSE_HANDLER_SEND_DEVICE_COMMAND_HPP_

#include <autom8/message/request_handler.hpp>
#include <autom8/device/device_base.hpp>

namespace autom8 {
    class send_device_command: public request_handler {
    private:
        typedef std::map<std::string, std::string> param_list;

    public:
        static request_handler_ptr create();
        virtual bool can_handle(session_ptr, message_ptr);
        virtual void operator()(session_ptr, message_ptr);

    private:
        send_device_command();

        void dispatch(
            device_ptr device,
            const std::string& command,
            const param_list& params);
    };
}

#endif