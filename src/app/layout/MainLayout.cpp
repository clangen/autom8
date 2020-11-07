#include "MainLayout.h"
#include <app/util/Message.h>
#include <cursespp/Colors.h>
#include <cursespp/Screen.h>
#include <f8n/debug/debug.h>

using namespace autom8;
using namespace autom8::app;
using namespace cursespp;
using namespace f8n;
using namespace f8n::runtime;

static const int REGISTER_FOR_BROADCASTS = app::message::CreateType();
static const int UPDATE_STATUS_MESSAGE = app::message::CreateType();
static const int SCHEDULE_RECONNECT = app::message::CreateType();

MainLayout::MainLayout(App& app, ConsoleLogger* logger, client_ptr client)
: AppLayout(app)
, client(client) {
    this->clientLayout = std::make_shared<ClientLayout>(client);
    this->settingsLayout = std::make_shared<SettingsLayout>(client);
    this->consoleLayout = std::make_shared<ConsoleLayout>(logger);

    this->clientStatus = std::make_shared<TextLabel>();
    this->clientStatus->SetText("client: disconnected", text::AlignCenter);
    this->clientStatus->SetContentColor(Color::Banner);
    this->AddWindow(clientStatus);

    this->serverStatus = std::make_shared<TextLabel>();
    this->serverStatus->SetText("server: stopped", text::AlignCenter);
    this->serverStatus->SetContentColor(Color::Banner);
    this->AddWindow(serverStatus);

    client->state_changed.connect(this, &MainLayout::OnClientStateChanged);
    autom8::server::started.connect(this, &MainLayout::OnServerStateChanged);
    autom8::server::stopped.connect(this, &MainLayout::OnServerStateChanged);

    this->SetLayout(this->clientLayout);
    this->Post(REGISTER_FOR_BROADCASTS);
    this->UpdateStatus();
}

MainLayout::~MainLayout() {
    MessageQueue().UnregisterForBroadcasts(this);
}

void MainLayout::OnLayout() {
    bool serverRunning = autom8::server::is_running();
    this->SetPadding(serverRunning ? 2 : 1, 0, 0, 0);
    AppLayout::OnLayout();
    auto cx = Screen::GetWidth();
    this->clientStatus->MoveAndResize(0, 0, cx, 1);
    if (serverRunning) {
        this->serverStatus->MoveAndResize(0, 1, cx, 1);
    }
}

void MainLayout::ProcessMessage(f8n::runtime::IMessage& message) {
    if (message.Type() == REGISTER_FOR_BROADCASTS) {
        MessageQueue().RegisterForBroadcasts(shared_from_this());
    }
    else if (message.Type() == message::BROADCAST_SWITCH_TO_SETTINGS_LAYOUT) {
        this->SetLayout(this->settingsLayout);
    }
    else if (message.Type() == message::BROADCAST_SWITCH_TO_CLIENT_LAYOUT) {
        this->SetLayout(this->clientLayout);
    }
    else if (message.Type() == message::BROADCAST_SWITCH_TO_CONSOLE_LAYOUT) {
        this->SetLayout(this->consoleLayout);
    }
    if (message.Type() == UPDATE_STATUS_MESSAGE) {
        this->UpdateStatus();
    }
    else if (message.Type() == SCHEDULE_RECONNECT) {
        if (client->state() == autom8::client::state_disconnected) {
            debug::info("ClientLayout", "reconnecting...");
            client->reconnect();
        }
    }
    else {
        AppLayout::ProcessMessage(message);
    }
}

void MainLayout::UpdateStatus() {
    using S = autom8::client::connection_state;

    std::string str = "disconnected";
    auto color = Color::TextDisabled;
    switch (client->state()) {
        case S::state_connecting:
            str = "connecting";
            break;
        case S::state_connected:
            str = std::string("connected to ") + client->hostname();
            color = Color::Header;
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
        this->serverStatus->Show();
    }
    else {
        this->serverStatus->Hide();
    }

    this->serverStatus->SetText(std::string("local server ") + str, cursespp::text::AlignCenter);
    this->serverStatus->SetContentColor(Color(color));
    this->Layout();
}

void MainLayout::OnServerStateChanged() {
    this->Post(UPDATE_STATUS_MESSAGE);
}

void MainLayout::OnClientStateChanged(client::connection_state state, client::reason reason) {
    this->Post(UPDATE_STATUS_MESSAGE);

    if (state == autom8::client::state_disconnected) {
        if (reason != client::auth_failed && reason != client::user) {
            debug::info("ClientLayout", "scheduling reconnect...");
            this->Remove(SCHEDULE_RECONNECT);
            this->Post(SCHEDULE_RECONNECT, 0L, 0L, 5000);
        }
    }
}