#pragma once

#include <autom8/message/request_handler.hpp>

namespace autom8 {
    class request_handler_registrar{
    private:
        request_handler_registrar() { } // no instances allowed.

    public:
        static void register_all();
    };
}
