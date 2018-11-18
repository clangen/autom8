#include "ClientLayout.h"
#include <cursespp/App.h>
#include <cursespp/Colors.h>
#include <cursespp/Screen.h>
#include <f8n/debug/debug.h>
#include <f8n/i18n/Locale.h>
#include <app/util/Device.h>
#include <app/util/Message.h>

using namespace autom8;
using namespace autom8::app;
using namespace cursespp;
using namespace f8n;
using namespace f8n::runtime;

static const int UPDATE_STATUS_MESSAGE = app::message::CreateType();
static const int SCHEDULE_RECONNECT = app::message::CreateType();

ClientLayout::ClientLayout(client_ptr client)
: LayoutBase()
, client(client) {
    this->deviceListAdapter = std::make_shared<DeviceListAdapter>(
        client, Window::MessageQueue(), [this]() {
            this->deviceList->OnAdapterChanged();
        });

    this->clientStatus = std::make_shared<TextLabel>();
    this->clientStatus->SetText("client: disconnected", text::AlignCenter);
    this->clientStatus->SetContentColor(Color::Banner);
    this->AddWindow(clientStatus);

    this->serverStatus = std::make_shared<TextLabel>();
    this->serverStatus->SetText("server: stopped", text::AlignCenter);
    this->serverStatus->SetContentColor(Color::Banner);
    this->AddWindow(serverStatus);

    this->deviceList = std::make_shared<ListWindow>(this->deviceListAdapter);
    this->deviceList->SetFrameTitle("devices");
    this->deviceList->EntryActivated.connect(this, &ClientLayout::OnDeviceRowActivated);
    this->AddWindow(this->deviceList);
    this->deviceList->SetFocusOrder(0);

    client->state_changed.connect(this, &ClientLayout::OnClientStateChanged);
    autom8::server::started.connect(this, &ClientLayout::OnServerStateChanged);
    autom8::server::stopped.connect(this, &ClientLayout::OnServerStateChanged);

    this->Update();
}

void ClientLayout::SetShortcutsWindow(cursespp::ShortcutsWindow* shortcuts) {
    debug::info("ClientLayout", "SetShortcutsWindow()");

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
    this->clientStatus->MoveAndResize(0, 0, cx, 1);
    this->serverStatus->MoveAndResize(0, 1, cx, 1);
    this->deviceList->MoveAndResize(0, 2, cx, cy - 2);
}

void ClientLayout::ProcessMessage(IMessage& message){
    if (message.Type() == UPDATE_STATUS_MESSAGE) {
        this->Update();
    }
    else if (message.Type() == SCHEDULE_RECONNECT) {
        if (client->state() == autom8::client::state_disconnected) {
            debug::info("ClientLayout", "reconnecting...");
            client->reconnect();
        }
    }
}

void ClientLayout::Update() {
    using S = autom8::client::connection_state;

    std::string str = "disconnected";
    auto color = Color::TextDisabled;
    switch (client->state()) {
        case S::state_connecting:
            str = "connecting";
            break;
        case S::state_connected:
            str = std::string("connected to ") + client->hostname();
            color = Color::Banner;
            break;
        case S::state_disconnecting:
            str = "disconnecting";
            break;
        default:
            break;
    }

    this->clientStatus->SetText(str, cursespp::text::AlignCenter);
    this->clientStatus->SetContentColor(Color(color));

    str = "stopped";
    color = Color::TextDisabled;
    if (autom8::server::is_running()) {
        str = "running";
        color = Color::ListItemHeader;
    }

    this->serverStatus->SetText(std::string("local server ") + str, cursespp::text::AlignCenter);
    this->serverStatus->SetContentColor(Color(color));
}

void ClientLayout::OnDeviceRowActivated(ListWindow* listWindow, size_t index) {
    auto device = this->deviceListAdapter->At(index);
    device::Toggle(this->client, device);
}

bool ClientLayout::KeyPress(const std::string& kn) {
    if (kn == "s") {
        Broadcast(message::BROADCAST_SWITCH_TO_SETTINGS_LAYOUT);
        return true;
    }
    return LayoutBase::KeyPress(kn);
}

void ClientLayout::OnServerStateChanged() {
    this->Post(UPDATE_STATUS_MESSAGE);
}

void ClientLayout::OnClientStateChanged(autom8::client::connection_state state, autom8::client::reason reason) {
    this->Post(UPDATE_STATUS_MESSAGE);

    if (state == autom8::client::state_disconnected) {
        if (reason != autom8::client::auth_failed && reason != autom8::client::user) {
            debug::info("ClientLayout", "scheduling reconnect...");
            this->Remove(SCHEDULE_RECONNECT);
            this->Post(SCHEDULE_RECONNECT, 0L, 0L, 5000);
        }
    }
}