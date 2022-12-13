#include <f8n/daemon/daemon.h>
#include <f8n/environment/Environment.h>
#include <f8n/preferences/Preferences.h>
#include <f8n/debug/debug.h>
#include <f8n/str/util.h>

#include <autom8/autom8.h>
#include <autom8/version.h>
#include <autom8/net/client.hpp>
#include <autom8/net/server.hpp>
#include <autom8/device/device_system.hpp>

using namespace f8n;
using namespace f8n::prefs;

struct Daemon: f8n::daemon::Daemon {
    std::string Name() override {
        return "autom8d";
    }

    std::string Version() override {
        return f8n::str::format("%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    }

    std::string Hash() override {
        return VERSION_COMMIT_HASH;
    }

    std::string LockFilename() override {
        return "/tmp/autom8d.lock";
    }

    std::vector<int> GetSignals() override {
        return { SIGUSR1 };
    }

    void OnInit(f8n::daemon::Type type, f8n::runtime::MessageQueue& messageQueue) override {
        if (type == f8n::daemon::Type::Background) {
            freopen("/tmp/autom8d.log", "w", stderr);
            debug::Start({
                new debug::SimpleFileBackend()
            });
        }
        else {
            debug::Start({
                new debug::ConsoleBackend(),
                new debug::SimpleFileBackend()
            });
        }

        f8n::env::Initialize("autom8", ENVIRONMENT_VERSION);
        auto prefs = f8n::prefs::Preferences::ForComponent("settings");
        auto systemType = prefs->GetString("server.controller");
        auto serverPort = prefs->GetInt("server.port", 7901);
        debug::i("daemon", f8n::str::format(
            "system type: '%s' port: '%d'",
            systemType.c_str(),
            serverPort));
        autom8::device_system::set_instance(systemType);
        autom8::server::start(serverPort);
    }

    void OnDeinit() override{
        autom8::device_system::clear_instance();
        if (autom8::server::is_running()) {
            autom8::server::stop();
        }
        debug::Stop();
    }

    void OnSignal(int signal) override {
        debug::i("daemon", f8n::str::format("signal received: %d", signal));
    }
} instance;

int main (int argc, char** argv) {
    f8n::daemon::start(argc, argv, instance);
}