#ifndef __C_AUTOM8_REQUEST_HANDLER_HPP__
#define __C_AUTOM8_REQUEST_HANDLER_HPP__

#include <autom8/message/message.hpp>

namespace autom8 {
    class session;

    class request_handler {
    protected:
        typedef boost::shared_ptr<session> session_ptr;

    public:
        virtual bool can_handle(session_ptr, message_ptr) = 0;
        virtual void operator()(session_ptr, message_ptr) = 0;
    };

    typedef boost::shared_ptr<request_handler> request_handler_ptr;
}

#endif