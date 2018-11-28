#include "ClientLayout.h"
#include <f8n/debug/debug.h>
#include <f8n/i18n/Locale.h>
#include <f8n/sdk/ISchema.h>
#include <cursespp/App.h>
#include <cursespp/InputOverlay.h>
#include <cursespp/NumberValidator.h>
#include <cursespp/Screen.h>
#include <app/util/Device.h>
#include <app/util/Message.h>

using namespace autom8;
using namespace autom8::app;
using namespace cursespp;
using namespace f8n;
using namespace f8n::runtime;
using namespace f8n::sdk;

ClientLayout::ClientLayout(client_ptr client)
: LayoutBase()
, client(client) {
    this->deviceListAdapter = std::make_shared<DeviceListAdapter>(
        client, Window::MessageQueue(), [this]() {
            this->deviceList->OnAdapterChanged();
        });

    this->deviceList = std::make_shared<ListWindow>(this->deviceListAdapter);
    this->deviceList->SetFrameTitle(_TSTR("device_list_title"));
    this->deviceList->EntryActivated.connect(this, &ClientLayout::OnDeviceRowActivated);
    this->deviceList->EntryContextMenu.connect(this, &ClientLayout::OnDeviceRowContextMenu);
    this->AddWindow(this->deviceList);
    this->deviceList->SetFocusOrder(0);
}

void ClientLayout::SetShortcutsWindow(cursespp::ShortcutsWindow* shortcuts) {
    debug::info("ClientLayout", "SetShortcutsWindow()");

this->shortcuts = shortcuts;

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

        shortcuts->SetActive("d");
    }
}

void ClientLayout::OnLayout() {
    LayoutBase::OnLayout();
    int cx = this->GetContentWidth();
    int cy = this->GetContentHeight();
    this->deviceList->MoveAndResize(0, 0, cx, cy);
}

void ClientLayout::OnDeviceRowActivated(ListWindow* listWindow, size_t index) {
    auto device = this->deviceListAdapter->At(index);
    device::Toggle(this->client, device);
}

void ClientLayout::OnDeviceRowContextMenu(cursespp::ListWindow* w, size_t index) {
    static const int MIN_BRIGHTNESS = 0;
    static const int MAX_BRIGHTNESS = 100;

    auto device = this->deviceListAdapter->At(index);
    if (device::Type(device) == autom8::device_type_lamp) {
        int brightness = device
            .value("attributes", nlohmann::json::object())
            .value("brightness", MAX_BRIGHTNESS);

        auto title = str::format(
            _TSTR("device_lamp_brightness_overlay_title"),
            MIN_BRIGHTNESS, MAX_BRIGHTNESS);

        auto validator = std::make_shared<NumberValidator<int>>(
            MIN_BRIGHTNESS,  MAX_BRIGHTNESS, [](int value) -> std::string {
                return std::to_string(value);
            });

        size_t width = (size_t)(Screen::GetWidth() * 0.8);

        std::shared_ptr<InputOverlay> dialog(new InputOverlay());
        auto client = this->client;

        dialog->SetTitle(title)
            .SetText(std::to_string(brightness))
            .SetValidator(validator)
            .SetWidth(width)
            .SetInputAcceptedCallback([client, device, brightness](std::string value) {
                int updated = std::stoi(value);
                if (updated != brightness) {
                    device::SetBrightness(client, device, updated);
                }
            });

        App::Overlays().Push(dialog);
    }
}


bool ClientLayout::KeyPress(const std::string& kn) {
    if (kn == "s") {
        Broadcast(message::BROADCAST_SWITCH_TO_SETTINGS_LAYOUT);
        return true;
    }
    return LayoutBase::KeyPress(kn);
}
