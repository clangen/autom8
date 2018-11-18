#include "ClientLayout.h"
#include <cursespp/Colors.h>
#include <cursespp/Screen.h>
#include <f8n/debug/debug.h>
#include <app/util/Device.h>

using namespace autom8;
using namespace autom8::app;
using namespace cursespp;
using namespace f8n;
using namespace f8n::runtime;

static const int UPDATE_STATUS_MESSAGE = 1024;
static const int SCHEDULE_RECONNECT = 1025;

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
    this->AddWindow(this->deviceList);
    this->deviceList->SetFocusOrder(0);

    client->state_changed.connect(this, &ClientLayout::OnClientStateChanged);
    autom8::server::started.connect(this, &ClientLayout::OnServerStateChanged);
    autom8::server::stopped.connect(this, &ClientLayout::OnServerStateChanged);

    this->Update();
}

void ClientLayout::SetShortcutsWindow(cursespp::ShortcutsWindow* w) {
    debug::info("ClientLayout", "SetShortcutsWindow()");
}

void ClientLayout::OnLayout() {
    LayoutBase::OnLayout();
    int cx = this->GetContentWidth();
    int cy = this->GetContentHeight();
    this->clientStatus->MoveAndResize(0, 0, cx, 1);
    this->serverStatus->MoveAndResize(0, cy - 1, cx, 1);
    this->deviceList->MoveAndResize(0, 1, cx, cy - 2);
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
            color = Color::TextActive;
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
        color = Color::TextWarning;
    }

    this->serverStatus->SetText(std::string("server ") + str, cursespp::text::AlignCenter);
    this->serverStatus->SetContentColor(Color(color));
}

bool ClientLayout::KeyPress(const std::string& kn) {
    if (kn == "KEY_ENTER") {
        size_t index = this->deviceList->GetSelectedIndex();
        if (index != ListWindow::NO_SELECTION) {
            auto device = this->deviceListAdapter->At(index);
            device::Toggle(this->client, device);
        }
        return true;
    }
    return false;
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