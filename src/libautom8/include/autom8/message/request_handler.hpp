#pragma once

#include <autom8/message/message.hpp>

namespace autom8 {
    class session;

    class request_handler {
    protected:
        typedef std::shared_ptr<session> session_ptr;

    public:
        virtual bool can_handle(session_ptr, message_ptr) = 0;
        virtual void operator()(session_ptr, message_ptr) = 0;
    };

    typedef std::shared_ptr<request_handler> request_handler_ptr;
}
