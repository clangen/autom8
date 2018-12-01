#include "Settings.h"
#include <autom8/device/device_system.hpp>
#include <cursespp/Colors.h>

using namespace autom8;
using namespace f8n::sdk;
using namespace f8n::prefs;

namespace autom8 { namespace app { namespace settings {

    const std::string CLIENT_HOSTNAME = "client.hostname";
    const std::string CLIENT_PASSWORD = "client.password";
    const std::string CLIENT_PORT = "client.port";
    const std::string SERVER_ENABLED = "server.enabled";
    const std::string SERVER_PASSWORD = "server.password";
    const std::string SERVER_PORT = "server.port";
    const std::string SERVER_CONTROLLER = "server.controller";
    const std::string UI_BACKGROUND_TYPE = "ui.backgroundType";
    const std::string UI_COLOR_MODE = "ui.colorMode";
    const std::string UI_THEME = "ui.theme";

    static const std::string DEFAULT_UI_BACKGROUND_TYPE = "theme";
    static const std::string DEFAULT_UI_THEME = "default";

#ifdef WIN32
    static const std::string DEFAULT_UI_COLOR_MODE = "rgb";
#else
    static const std::string DEFAULT_UI_COLOR_MODE = "palette";
#endif

    void InitializeDefaults() {
        static bool initialized = false;
        if (!initialized) {
            initialized = true;
            auto prefs = Prefs();
            prefs->SetDefault(CLIENT_HOSTNAME, "localhost");
            prefs->SetDefault(CLIENT_PASSWORD, "changeme");
            prefs->SetDefault(CLIENT_PORT, 7901);
            prefs->SetDefault(SERVER_ENABLED, true);
            prefs->SetDefault(SERVER_PASSWORD, "changeme");
            prefs->SetDefault(SERVER_PORT, 7901);
            prefs->SetDefault(SERVER_CONTROLLER, device_system::default_type());
            prefs->SetDefault(UI_BACKGROUND_TYPE, DEFAULT_UI_BACKGROUND_TYPE);
            prefs->SetDefault(UI_COLOR_MODE, DEFAULT_UI_COLOR_MODE);
            prefs->SetDefault(UI_THEME, DEFAULT_UI_THEME);
        }
    }

    std::shared_ptr<ISchema> Schema() {
        auto result = std::make_shared<TSchema<>>();
        result->Add(ClientSchema().get());
        result->Add(ServerSchema().get());
        result->Add(UiSchema().get());
        return result;
    }

    std::shared_ptr<ISchema> UiSchema() {
        auto result = std::make_shared<TSchema<>>();
        result->AddEnum(
            UI_BACKGROUND_TYPE,
            { "inherit", "theme" },
            DEFAULT_UI_BACKGROUND_TYPE);
        result->AddEnum(
            UI_COLOR_MODE,
            { "basic", "rgb", "palette" },
            DEFAULT_UI_COLOR_MODE);
        result->AddEnum(
            UI_THEME,
            cursespp::Colors::ListThemes(),
            DEFAULT_UI_THEME);
        return result;
    }

    std::shared_ptr<ISchema> ClientSchema() {
        auto result = std::make_shared<TSchema<>>();
        result->AddString(CLIENT_HOSTNAME);
        result->AddString(CLIENT_PASSWORD);
        result->AddInt(CLIENT_PORT);
        return result;
    }

    std::shared_ptr<ISchema> ServerSchema() {
        auto result = std::make_shared<TSchema<>>();
        result->AddBool(SERVER_ENABLED);
        result->AddString(SERVER_PASSWORD);
        result->AddInt(SERVER_PORT);
        result->AddEnum(
            SERVER_CONTROLLER,
            device_system::types(),
            device_system::default_type());
        return result;
    }

    std::shared_ptr<Preferences> Prefs() {
        return Preferences::ForComponent("settings");
    }

    cursespp::Colors::BgType BackgroundType() {
        auto value = Prefs()->GetString(UI_BACKGROUND_TYPE);
        if (value == "inherit") {
            return cursespp::Colors::Inherit;
        }
        return cursespp::Colors::Theme;
    }

    cursespp::Colors::Mode ColorMode() {
        auto value = Prefs()->GetString(UI_COLOR_MODE);
        if (value == "rgb") {
            return cursespp::Colors::RGB;
        }
        else if (value == "palette") {
            return cursespp::Colors::Palette;
        }
        return cursespp::Colors::Basic;
    }

}}}