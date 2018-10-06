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

#include <cursespp/App.h>
#include <cursespp/Screen.h>
#include <cursespp/TextLabel.h>
#include <cursespp/LayoutBase.h>
#include <cursespp/IViewRoot.h>

#include <boost/locale.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>

static const std::string APP_NAME = "autom8";
static const int MAX_SIZE = 1000;
static const int DEFAULT_WIDTH = 100;
static const int MIN_WIDTH = 54;
static const int DEFAULT_HEIGHT = 26;
static const int MIN_HEIGHT = 12;

using namespace cursespp;

static void initUtf8Filesystem() {
    std::locale locale = std::locale();
    std::locale utf8Locale(locale, new boost::filesystem::detail::utf8_codecvt_facet);
    boost::filesystem::path::imbue(utf8Locale);
}

class MainLayout: public LayoutBase, public IViewRoot {
    public:
        MainLayout() : LayoutBase() {
            this->label = std::make_shared<TextLabel>();
            this->label->SetText("hello, autom8", text::AlignCenter);
            this->SetFrameVisible(true);
            this->SetFrameTitle("devices");
            this->AddWindow(label);
        }

        virtual void ResizeToViewport() override {
            this->MoveAndResize(0, 0, Screen::GetWidth(), Screen::GetHeight());
        }

        virtual void OnLayout() override {
            this->label->MoveAndResize(0, this->GetContentHeight() / 2, this->GetContentWidth(), 1);
        }

    private:
        std::shared_ptr<TextLabel> label;
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

    App app(APP_NAME); /* must be before layout creation */

    app.SetMinimumSize(MIN_WIDTH, MIN_HEIGHT);

    app.SetKeyHandler([&](const std::string& kn) -> bool {
        return false;
    });

    app.Run(std::make_shared<MainLayout>());

    return 0;
}
