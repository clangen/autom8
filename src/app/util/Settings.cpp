#include "Settings.h"
#include <autom8/device/device_system.hpp>

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
        }
    }

    std::shared_ptr<ISchema> Schema() {
        auto result = std::make_shared<TSchema<>>();
        result->Add(ClientSchema().get());
        result->Add(ServerSchema().get());
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

}}}