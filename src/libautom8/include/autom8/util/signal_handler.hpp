#ifndef __C_AUTOM8_SIGNAL_HANDLER_HPP__
#define __C_AUTOM8_SIGNAL_HANDLER_HPP__

#include <sigslot/sigslot.h>

namespace autom8 {
    typedef sigslot::has_slots<> signal_handler;
}

#endif