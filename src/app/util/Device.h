#pragma once

#include <json.hpp>
#include <autom8/net/client.hpp>

namespace autom8 { namespace app { namespace device {

    bool Toggle(autom8::client_ptr, nlohmann::json& device);

} } }