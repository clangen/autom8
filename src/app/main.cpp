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

#include <boost/locale.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>

#include <autom8/autom8.h>
#include <autom8/net/client.hpp>
#include <autom8/net/server.hpp>
#include <autom8/util/utility.hpp>

#include <cursespp/App.h>
#include <cursespp/Screen.h>
#include <cursespp/TextLabel.h>
#include <cursespp/LayoutBase.h>
#include <cursespp/IViewRoot.h>

static const std::string APP_NAME = "autom8";
static const int MAX_SIZE = 1000;
static const int DEFAULT_WIDTH = 100;
static const int MIN_WIDTH = 54;
static const int DEFAULT_HEIGHT = 26;
static const int MIN_HEIGHT = 12;

using namespace cursespp;

using client_ptr = std::shared_ptr<autom8::client>;

static void initUtf8Filesystem() {
    std::locale locale = std::locale();
    std::locale utf8Locale(locale, new boost::filesystem::detail::utf8_codecvt_facet);
    boost::filesystem::path::imbue(utf8Locale);
}

class MainLayout: public LayoutBase, public IViewRoot, public sigslot::has_slots<> {
    public:
        MainLayout(client_ptr client)
        : LayoutBase()
        , client(client) {
            this->status = std::make_shared<TextLabel>();
            this->status->SetText("disconnected", text::AlignCenter);
            this->status->SetContentColor(CURSESPP_BANNER);
            this->AddWindow(status);

            this->label = std::make_shared<TextLabel>();
            this->label->SetText("hello, autom8", text::AlignCenter);
            this->AddWindow(label);

            client->state_changed.connect(this, &MainLayout::OnStateChanged);
            this->Update();
        }

        virtual void ResizeToViewport() override {
            this->MoveAndResize(0, 0, Screen::GetWidth(), Screen::GetHeight());
        }

        virtual void OnLayout() override {
            int cx = this->GetContentWidth();
            this->status->MoveAndResize(0, 0, cx, 1);
            this->label->MoveAndResize(0, this->GetContentHeight() / 2, cx, 1);
        }

    private:
        void Update() {
            using S = autom8::client::connection_state;

            auto str = "disconnected";
            auto color = CURSESPP_BANNER;
            switch (client->state()) {
                case S::state_connecting:
                    str = "connecting";
                    break;
                case S::state_connected:
                    str = "connected";
                    color = CURSESPP_FOOTER;
                    break;
                case S::state_disconnecting:
                    str = "disconnecting";
                    break;
            }

            this->status->SetText(str, cursespp::text::AlignCenter);
            this->status->SetContentColor(color);
        }

        void OnConnected() {
            this->Update();
        }

        void OnStateChanged(autom8::client::connection_state state, autom8::client::reason reason) {
            this->Update();
        }

        client_ptr client;
        std::shared_ptr<TextLabel> label;
        std::shared_ptr<TextLabel> status;
};

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
    srand((unsigned int)time(0));
    initUtf8Filesystem();

    autom8::server::start(7901);

    std::string password = "changeme";
    std::string host = "localhost";
    std::string port = "7901";
    std::string hashed = autom8::utility::sha256(password.c_str(), password.size());
    auto client = std::make_shared<autom8::client>(host, port);
    client->connect(hashed);

    App app(APP_NAME); /* must be before layout creation */

    app.SetMinimumSize(MIN_WIDTH, MIN_HEIGHT);

    app.SetKeyHandler([&](const std::string& kn) -> bool {
        if (kn == "d") {
            client->disconnect();
            return true;
        }
        else if (kn == "r") {
            client->connect(hashed);
            return true;
        }
        return false;
    });

    app.Run(std::make_shared<MainLayout>(client));

    autom8::server::stop();

    return 0;
}
