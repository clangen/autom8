#pragma once

#include <f8n/sdk/ISchema.h>
#include <f8n/preferences/Preferences.h>
#include <memory>

namespace autom8 { namespace app { namespace settings {

    void InitializeDefaults();
    std::shared_ptr<f8n::sdk::ISchema> Schema();
    std::shared_ptr<f8n::prefs::Preferences> Prefs();

    extern const std::string CLIENT_HOSTNAME;
    extern const std::string CLIENT_PASSWORD;
    extern const std::string CLIENT_PORT;
    extern const std::string SERVER_PASSWORD;
    extern const std::string SERVER_PORT;
    extern const std::string SYSTEM_SELECTED;
    extern const std::string MOCHAD_HOSTNAME;
    extern const std::string MOCHAD_PORT;

} } }