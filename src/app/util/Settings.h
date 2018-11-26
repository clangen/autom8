#pragma once

#include <f8n/sdk/ISchema.h>
#include <f8n/preferences/Preferences.h>
#include <cursespp/Colors.h>
#include <memory>

namespace autom8 { namespace app { namespace settings {

    void InitializeDefaults();
    std::shared_ptr<f8n::sdk::ISchema> Schema();
    std::shared_ptr<f8n::sdk::ISchema> ClientSchema();
    std::shared_ptr<f8n::sdk::ISchema> ServerSchema();
    std::shared_ptr<f8n::prefs::Preferences> Prefs();
    cursespp::Colors::BgType BackgroundType();
    cursespp::Colors::Mode ColorMode();

    extern const std::string CLIENT_HOSTNAME;
    extern const std::string CLIENT_PASSWORD;
    extern const std::string CLIENT_PORT;
    extern const std::string SERVER_ENABLED;
    extern const std::string SERVER_PASSWORD;
    extern const std::string SERVER_PORT;
    extern const std::string SERVER_CONTROLLER;
    extern const std::string UI_BACKGROUND_TYPE;
    extern const std::string UI_COLOR_MODE;

} } }