#ifndef __C_AUTOM8_REQUEST_HANDLER_REGISTRAR_HPP__
#define __C_AUTOM8_REQUEST_HANDLER_REGISTRAR_HPP__

#include <autom8/message/request_handler.hpp>
#include <boost/thread.hpp>

namespace autom8 {
    class request_handler_registrar{
    private:
        request_handler_registrar() { } // no instances allowed.

    public:
        static void register_all();
    };
}

#endif