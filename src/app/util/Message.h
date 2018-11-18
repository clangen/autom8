#pragma once

#include <json.hpp>
#include <autom8/net/client.hpp>

namespace autom8 { namespace app { namespace message {

    int64_t CreateType();

    extern const int64_t BROADCAST_SWITCH_TO_CLIENT_LAYOUT;
    extern const int64_t BROADCAST_SWITCH_TO_SETTINGS_LAYOUT;

} } }