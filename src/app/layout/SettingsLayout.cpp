#include "SettingsLayout.h"
#include <f8n/debug/debug.h>
#include <f8n/i18n/Locale.h>
#include <cursespp/App.h>
#include <cursespp/Screen.h>
#include <cursespp/PluginOverlay.h>
#include <autom8/device/device_system.hpp>
#include <app/util/Message.h>
#include <app/util/Settings.h>

using namespace cursespp;
using namespace autom8::app;
using namespace f8n;

SettingsLayout::SettingsLayout(autom8::client_ptr client)
: LayoutBase()
, client(client) {
    this->aboutConfig = std::make_shared<TextLabel>();
    this->aboutConfig->SetText(_TSTR("settings_about_config"));
    this->aboutConfig->SetFocusOrder(0);
    this->aboutConfig->Activated.connect(this, &SettingsLayout::OnAboutConfigActivated);
    this->AddWindow(aboutConfig);

    this->deviceModelAdapter = std::make_shared<DeviceModelAdapter>(device_system::instance());
    this->deviceModelList = std::make_shared<ListWindow>(this->deviceModelAdapter);
    this->deviceModelList->SetFrameTitle(_TSTR("settings_device_system_list"));
    this->deviceModelList->SetFocusOrder(1);
    this->AddWindow(this->deviceModelList);
}

void SettingsLayout::OnLayout() {
    auto cx = this->GetContentWidth();
    this->aboutConfig->MoveAndResize(1, 1, cx - 2, 1);
    this->deviceModelList->MoveAndResize(1, 3, cx - 2, 8);
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

void SettingsLayout::OnAboutConfigActivated(cursespp::TextLabel* label) {
    PluginOverlay::Show(
        "settings",
        settings::Prefs(),
        settings::Schema(),
        [this](bool changed) {
            if (changed) {
                debug::i("ClientLayout", "settings saved, reconnecting...");
                auto prefs = settings::Prefs();
                prefs->Save();
                this->client->disconnect(true);
                this->client->connect(
                    prefs->GetString(settings::CLIENT_HOSTNAME),
                    prefs->GetInt(settings::CLIENT_PORT),
                    prefs->GetString(settings::CLIENT_PASSWORD));
            }
        });
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