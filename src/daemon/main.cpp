#include <f8n/daemon/daemon.h>
#include <f8n/environment/Environment.h>
#include <f8n/preferences/Preferences.h>
#include <f8n/debug/debug.h>

#include <autom8/autom8.h>
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
        return "0.90.0";
    }

    std::string Hash() override {
        return "nohash";
    }

    std::string LockFilename() override {
        return "/tmp/autom8d.lock";
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
    }

    void OnDeinit() override{
        autom8::device_system::clear_instance();
        if (autom8::server::is_running()) {
            autom8::server::stop();
        }
        debug::Stop();
    }

    void OnSignal(int signal) override {
    }
} instance;

int main (int argc, char** argv) {
    f8n::daemon::start(argc, argv, instance);
}