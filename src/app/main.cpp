//////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2007-2017 musikcube team
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the author nor the names of other contributors may
//      be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WINVER
#define WINVER 0x0502
#define _WIN32_WINNT 0x0502
#endif
#include <windows.h>
#endif

#include <autom8/autom8.h>
#include <autom8/net/client.hpp>
#include <autom8/net/server.hpp>
#include <autom8/util/utility.hpp>
#include <autom8/util/preferences.hpp>
#include <autom8/message/requests/get_device_list.hpp>
#include <autom8/device/device_system.hpp>
#include <autom8/device/null_device_system.hpp>

#include <cursespp/App.h>

#include <app/layout/MainLayout.h>

#include <f8n/debug/debug.h>
#include <f8n/environment/Environment.h>

static const std::string APP_NAME = "autom8";
static const int MAX_SIZE = 1000;
static const int DEFAULT_WIDTH = 100;
static const int MIN_WIDTH = 54;
static const int DEFAULT_HEIGHT = 26;
static const int MIN_HEIGHT = 12;

using namespace cursespp;
using namespace autom8::app;

using client_ptr = std::shared_ptr<autom8::client>;

#ifdef WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow) {
    PDC_set_resize_limits(MIN_HEIGHT, MAX_SIZE, MIN_WIDTH, MAX_SIZE);
    resize_term(DEFAULT_HEIGHT, DEFAULT_WIDTH); /* must be before app init */

    if (App::Running(APP_NAME)) {
        return 0;
    }
#else
int main(int argc, char* argv[]) {
#endif

    f8n::env::Initialize(APP_NAME, 1);

    f8n::debug::start({
        new f8n::debug::FileBackend(f8n::env::GetDataDirectory() + "log.txt")
    });

    f8n::debug::info("main", "test!");

    std::string password = "changeme";
    std::string hashed = autom8::utility::sha256(password.c_str(), password.size());
    std::string host = "localhost";
    unsigned short port = 7901;

    autom8::utility::prefs().set("password", hashed);

    {
        using device_system_ptr = std::shared_ptr<autom8::device_system>;
        device_system_ptr system = std::make_shared<autom8::null_device_system>();
        std::vector<std::string> groups;
        system->model().add(autom8::device_type_appliance, "a1", "dummy appliance 1", groups);
        autom8::device_system::set_instance(system);

        autom8::server::start(7901);

        auto client = std::make_shared<autom8::client>();
        client->connect(host, port, hashed);

        App app(APP_NAME); /* must be before layout creation */

        app.SetMinimumSize(MIN_WIDTH, MIN_HEIGHT);

        app.SetKeyHandler([&](const std::string& kn) -> bool {
            if (kn == "d") {
                client->disconnect();
                return true;
            }
            else if (kn == "c") {
                client->reconnect(hashed);
                return true;
            }
            else if (kn == "s") {
                autom8::server::stop();
                return true;
            }
            else if (kn == "r") {
                autom8::server::start(7901);
                return true;
            }
            else if (kn == "l") {
                if (client->state() == autom8::client::state_connected) {
                    client->send(autom8::get_device_list::request());
                }
            }
            return false;
        });

        app.Run(std::make_shared<MainLayout>(client));
    }

    autom8::device_system::clear_instance(); /* ugh. fix this. */

    autom8::server::stop();

    f8n::debug::stop();

    return 0;
}
