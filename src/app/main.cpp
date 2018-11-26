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
#include <windows.h>
#endif

#include <autom8/autom8.h>
#include <autom8/net/client.hpp>
#include <autom8/net/server.hpp>
#include <autom8/device/device_system.hpp>

#include <cursespp/App.h>
#include <cursespp/Colors.h>

#include <app/layout/MainLayout.h>
#include <app/util/Settings.h>

#include <f8n/environment/Environment.h>
#include <f8n/preferences/Preferences.h>
#include <f8n/debug/debug.h>

#ifdef WIN32
#include "resource.h"
#endif

static const std::string APP_NAME = "autom8";
static const int MAX_SIZE = 1000;
static const int DEFAULT_WIDTH = 54;
static const int MIN_WIDTH = 42;
static const int DEFAULT_HEIGHT = 26;
static const int MIN_HEIGHT = 10;

using namespace cursespp;
using namespace f8n;
using namespace f8n::prefs;
using namespace autom8::app;

using client_ptr = std::shared_ptr<autom8::client>;

#ifdef WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow) {
    PDC_set_resize_limits(MIN_HEIGHT, MAX_SIZE, MIN_WIDTH, MAX_SIZE);
    PDC_resize_screen(DEFAULT_HEIGHT, DEFAULT_WIDTH);

    if (App::Running(APP_NAME)) {
        return 0;
    }
#else
int main(int argc, char* argv[]) {
#endif

    env::Initialize(APP_NAME, 1);
    debug::Start();
    debug::info("main", "app starting");

    settings::InitializeDefaults();
    auto prefs = settings::Prefs();

    {
        /* server startup */
        auto systemType = prefs->GetString(settings::SERVER_CONTROLLER);
        autom8::device_system::set_instance(systemType);
        if (prefs->GetBool(settings::SERVER_ENABLED)) {
            autom8::server::start(prefs->GetInt(settings::SERVER_PORT));
        }

        /* client startup */
        std::string password = prefs->GetString(settings::CLIENT_PASSWORD);
        std::string host = prefs->GetString(settings::CLIENT_HOSTNAME);
        int port = prefs->GetInt(settings::CLIENT_PORT);
        auto client = std::make_shared<autom8::client>();
        client->connect(host, port, password);

        App app(APP_NAME);

        app.SetMinimumSize(MIN_WIDTH, MIN_HEIGHT);
        app.SetColorMode(settings::ColorMode());
        app.SetColorBackgroundType(settings::BackgroundType());
       app.SetColorTheme(env::GetApplicationDirectory() + "themes/gruvbox_dark.json");

#ifdef WIN32
        app.SetIcon(IDI_ICON1);
        app.SetSingleInstanceId("autom8");
#endif

        app.SetKeyHandler([&](const std::string& kn) -> bool {
            if (kn == "M-d") {
                client->disconnect();
                return true;
            }
            else if (kn == "M-c") {
                client->reconnect();
                return true;
            }
            else if (kn == "M-x") {
                if (autom8::server::is_running()) {
                    autom8::server::stop();
                }
                else {
                    autom8::server::start(7901);
                }
                return true;
            }
            return false;
        });

        app.Run(std::make_shared<MainLayout>(app, client));

        client->disconnect();
    }

    autom8::device_system::clear_instance(); /* ugh. fix this. */

    if (autom8::server::is_running()) {
        autom8::server::stop();
    }
    debug::Stop();

    return 0;
}
