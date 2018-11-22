#include "Settings.h"

using namespace f8n::sdk;
using namespace f8n::prefs;

namespace autom8 { namespace app { namespace settings {

    const std::string CLIENT_HOSTNAME = "client.hostname";
    const std::string CLIENT_PASSWORD = "client.password";
    const std::string CLIENT_PORT = "client.port";
    const std::string SERVER_PASSWORD = "server.password";
    const std::string SERVER_PORT = "server.port";
    const std::string SYSTEM_SELECTED = "system.selected";
    const std::string MOCHAD_HOSTNAME = "mochad.hostname";
    const std::string MOCHAD_PORT = "mochad.port";

    void InitializeDefaults() {
        static bool initialized = false;
        if (!initialized) {
            initialized = true;
            auto prefs = Prefs();
            prefs->SetDefault(CLIENT_HOSTNAME, "localhost");
            prefs->SetDefault(CLIENT_PASSWORD, "changeme");
            prefs->SetDefault(CLIENT_PORT, 7901);
            prefs->SetDefault(SERVER_PASSWORD, "changeme");
            prefs->SetDefault(SERVER_PORT, 7901);
            prefs->SetDefault(SYSTEM_SELECTED, "null");
            prefs->SetDefault(MOCHAD_HOSTNAME, "localhost");
            prefs->SetDefault(MOCHAD_PORT, 1099);
        }
    }

    std::shared_ptr<ISchema> Schema() {
        auto result = std::make_shared<TSchema<>>();
        result->AddString(CLIENT_HOSTNAME);
        result->AddString(CLIENT_PASSWORD);
        result->AddInt(CLIENT_PORT);
        result->AddString(SERVER_PASSWORD);
        result->AddInt(SERVER_PORT);
        result->AddEnum(SYSTEM_SELECTED, { "null", "mochad" }, "null");
        result->AddString(MOCHAD_HOSTNAME);
        result->AddInt(MOCHAD_PORT);
        return result;
    }

    std::shared_ptr<Preferences> Prefs() {
        return Preferences::ForComponent("settings");
    }

}}}