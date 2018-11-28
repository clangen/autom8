#pragma once

#include <json.hpp>
#include <autom8/net/client.hpp>

namespace autom8 { namespace app { namespace device {

    bool Toggle(autom8::client_ptr, nlohmann::json& device);
    bool SetBrightness(autom8::client_ptr, const nlohmann::json& device, int brightness);
    autom8::device_type Type(const nlohmann::json& device);

} } }