#include "SettingsLayout.h"
#include <cursespp/App.h>
#include <app/util/Message.h>
#include <f8n/debug/debug.h>
#include <f8n/i18n/Locale.h>

using namespace cursespp;
using namespace autom8::app;
using namespace f8n;

SettingsLayout::SettingsLayout()
: LayoutBase() {

}

void SettingsLayout::OnLayout() {

}

void SettingsLayout::ProcessMessage(f8n::runtime::IMessage& message) {

}

bool SettingsLayout::KeyPress(const std::string& kn) {
    if (kn == "d") {
        Broadcast(message::BROADCAST_SWITCH_TO_CLIENT_LAYOUT);
        return true;
    }
    else {
        return LayoutBase::KeyPress(kn);
    }
}

void SettingsLayout::SetShortcutsWindow(cursespp::ShortcutsWindow* shortcuts) {
    debug::info("SettingsLayout", "SetShortcutsWindow()");

    if (shortcuts) {
        shortcuts->AddShortcut("d", _TSTR("shortcuts_devices"));
        shortcuts->AddShortcut("s", _TSTR("shortcuts_settings"));
        shortcuts->AddShortcut("^D", _TSTR("shortcuts_quit"));

        shortcuts->SetChangedCallback([this](std::string key) {
            if (key == "^D") {
                App::Instance().Quit();
            }
            else {
                this->KeyPress(key);
            }
        });

        shortcuts->SetActive("s");
    }
}