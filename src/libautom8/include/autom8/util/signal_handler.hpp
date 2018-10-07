#pragma once

#include <sigslot/sigslot.h>

namespace autom8 {
    typedef sigslot::has_slots<> signal_handler;
}
