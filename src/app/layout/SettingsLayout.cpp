#include "SettingsLayout.h"
#include <f8n/debug/debug.h>
#include <f8n/i18n/Locale.h>
#include <f8n/str/util.h>
#include <cursespp/App.h>
#include <cursespp/Colors.h>
#include <cursespp/SchemaOverlay.h>
#include <cursespp/Screen.h>
#include <autom8/device/device_system.hpp>
#include <app/util/Message.h>
#include <app/util/Settings.h>
#include <app/overlay/DeviceEditOverlay.h>

using namespace cursespp;
using namespace autom8;
using namespace autom8::app;
using namespace f8n;
using namespace f8n::sdk;

static bool isDeleteKey(const std::string& kn) {
#ifdef __APPLE__
    return kn == "KEY_BACKSPACE";
#else
    return kn == "KEY_DC";
#endif
}

SettingsLayout::SettingsLayout(client_ptr client)
: LayoutBase()
, client(client)
, schema(settings::Schema()) {
    int order = 0;

    this->clientConfig = std::make_shared<TextLabel>();
    this->clientConfig->SetText("> " + _TSTR("settings_configure_client"));
    this->clientConfig->SetFocusOrder(order++);
    this->clientConfig->Activated.connect(this, &SettingsLayout::OnConfigureClientActivated);
    this->AddWindow(clientConfig);

    this->uiConfig = std::make_shared<TextLabel>();
    this->uiConfig->SetText("> " + _TSTR("settings_configure_ui"));
    this->uiConfig->SetFocusOrder(order++);
    this->uiConfig->Activated.connect(this, &SettingsLayout::OnConfigureUiActivated);
    this->AddWindow(uiConfig);

    this->serverConfig = std::make_shared<TextLabel>();
    this->serverConfig->SetText("> " + _TSTR("settings_configure_server"));
    this->serverConfig->SetFocusOrder(order++);
    this->serverConfig->Activated.connect(this, &SettingsLayout::OnConfigureServerActivated);
    this->AddWindow(serverConfig);

    this->configureController = std::make_shared<TextLabel>();
    this->configureController->SetFocusOrder(order++);
    this->configureController->Activated.connect(this, &SettingsLayout::OnConfigureControllerActivated);
    this->AddWindow(configureController);

    this->addDevice = std::make_shared<TextLabel>();
    this->addDevice->SetFocusOrder(order++);
    this->addDevice->Activated.connect(this, &SettingsLayout::OnAddDeviceActivated);
    this->AddWindow(addDevice);

    this->deviceModelAdapter = std::make_shared<DeviceModelAdapter>(device_system::instance());
    this->deviceModelList = std::make_shared<ListWindow>(this->deviceModelAdapter);
    this->deviceModelList->SetAllowArrowKeyPropagation(true);
    this->deviceModelList->EntryActivated.connect(this, &SettingsLayout::OnDeviceRowActivated);
    this->deviceModelList->SetFocusOrder(order++);
    this->AddWindow(this->deviceModelList);

    this->ReloadController();
}

void SettingsLayout::OnLayout() {
    auto cx = this->GetContentWidth() - 1;
    auto cy = this->GetContentHeight();
    int x = 1;
    int y = 0;
    this->clientConfig->MoveAndResize(x, y++, cx, 1);
    this->uiConfig->MoveAndResize(x, y++, cx, 1);
    this->serverConfig->MoveAndResize(x, y++, cx, 1);
    this->configureController->MoveAndResize(x + 1, y++, cx - 1, 1);
    this->addDevice->MoveAndResize(x + 1, y++, cx - 1, 1);
    this->deviceModelList->MoveAndResize(1, y, cx, cy - y);
}

void SettingsLayout::ReloadController() {
    auto prefs = settings::Prefs();
    auto controller = prefs->GetString(settings::SERVER_CONTROLLER);

    device_system::set_instance(controller);

    this->configureController->SetText("> " + str::format(
        _TSTR("settings_configure_device_controller"), controller.c_str()));

    this->addDevice->SetText("> " + str::format(
        _TSTR("settings_add_device"), controller.c_str()));

    this->deviceModelList->SetFrameTitle(str::format(
        _TSTR("settings_device_system_list"), controller.c_str()));

    this->deviceModelAdapter->Requery(device_system::instance());
    this->deviceModelList->OnAdapterChanged();
}

void SettingsLayout::ReloadClient() {
    debug::i("ClientLayout", "settings saved, reconnecting...");
    this->ReloadController();
    auto prefs = settings::Prefs();
    prefs->Save();
    this->client->disconnect(true);
    this->client->connect(
        prefs->GetString(settings::CLIENT_HOSTNAME),
        prefs->GetInt(settings::CLIENT_PORT),
        prefs->GetString(settings::CLIENT_PASSWORD));
}

void SettingsLayout::ReloadUi() {
    Colors::Init(settings::ColorMode(), settings::BackgroundType());
    Colors::SetTheme(settings::Prefs()->GetString(settings::UI_THEME));
}

void SettingsLayout::ReloadServer() {
    this->ReloadController();

    auto prefs = settings::Prefs();
    bool enabled = prefs->GetBool(settings::SERVER_ENABLED);
    if (enabled && !server::is_running()) {
        server::start(prefs->GetInt(settings::SERVER_PORT));
    }
    else if (!enabled && server::is_running()) {
        server::stop();
    }
}

void SettingsLayout::OnDeviceRowActivated(ListWindow* window, size_t index) {
    DeviceEditOverlay::Edit(
        device_system::instance(),
        this->deviceModelAdapter->At(index),
        [this]() { this->ReloadController(); });
}

bool SettingsLayout::KeyPress(const std::string& kn) {
    if (kn == "d") {
        Broadcast(message::BROADCAST_SWITCH_TO_CLIENT_LAYOUT);
        return true;
    }
    else if (kn == "`" || kn == "~") {
        Broadcast(message::BROADCAST_SWITCH_TO_CONSOLE_LAYOUT);
        return true;
    }
    if (kn == "?") {
        SchemaOverlay::Show(
            _TSTR("settings_about_config"),
            settings::Prefs(),
            this->schema,
            [this](bool changed) {
                if (changed) {
                    this->ReloadClient();
                    this->ReloadUi();
                    this->ReloadServer();
                }
            });
    }
    else if (this->GetFocus() == this->deviceModelList && isDeleteKey(kn)) {
        auto index = this->deviceModelList->GetSelectedIndex();
        if (index != ListWindow::NO_SELECTION) {
            DeviceEditOverlay::Delete(
                device_system::instance(),
                this->deviceModelAdapter->At(index),
                [this]() { this->ReloadController(); });
        }
        return true;
    }

    return LayoutBase::KeyPress(kn);
}

void SettingsLayout::OnConfigureClientActivated(TextLabel* label) {
    SchemaOverlay::Show(
        _TSTR("settings_configure_client"),
        settings::Prefs(),
        settings::ClientSchema(),
        [this](bool changed) {
            if (changed) {
                this->ReloadClient();
            }
        });
}

void SettingsLayout::OnConfigureUiActivated(TextLabel* label) {
    SchemaOverlay::Show(
        _TSTR("settings_configure_ui"),
        settings::Prefs(),
        settings::UiSchema(),
        [this](bool changed) {
            if (changed) {
                this->ReloadUi();
            }
        });
}

void SettingsLayout::OnConfigureServerActivated(TextLabel* label) {
    SchemaOverlay::Show(
        _TSTR("settings_configure_server"),
        settings::Prefs(),
        settings::ServerSchema(),
        [this](bool changed) {
            if (changed) {
                this->ReloadServer();
            }
        });
}

void SettingsLayout::OnConfigureControllerActivated(TextLabel* label) {
    auto system = device_system::instance();
    auto schema = system->schema();
    if (schema && schema->Count()> 0) {
        auto prefs = settings::Prefs();
        std::string title = str::format(
            _TSTR("settings_configure_device_controller"),
            prefs->GetString(settings::SERVER_CONTROLLER).c_str());

        SchemaOverlay::Show(title, prefs, schema, [system](bool changed) {
            if (changed) {
                system->on_settings_changed();
            }
        });
    }
}

void SettingsLayout::OnAddDeviceActivated(TextLabel* label) {
    DeviceEditOverlay::Create(
        device_system::instance(),
        [this]() { this->ReloadController(); });
}

void SettingsLayout::SetShortcutsWindow(ShortcutsWindow* shortcuts) {
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