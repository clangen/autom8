#pragma once

#include <json.hpp>
#include <autom8/net/client.hpp>

namespace autom8 { namespace app { namespace message {

    int CreateType();

    extern const int BROADCAST_SWITCH_TO_CLIENT_LAYOUT;
    extern const int BROADCAST_SWITCH_TO_SETTINGS_LAYOUT;

} } }