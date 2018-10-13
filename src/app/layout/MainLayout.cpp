#include "MainLayout.h"

#include <cursespp/Colors.h>

using namespace autom8;
using namespace autom8::app;
using namespace cursespp;
using namespace f8n::runtime;

static const int UPDATE_STATUS_MESSAGE = 1024;

MainLayout::MainLayout(client_ptr client)
: LayoutBase()
, client(client) {
    this->clientStatus = std::make_shared<TextLabel>();
    this->clientStatus->SetText("client: disconnected", text::AlignCenter);
    this->clientStatus->SetContentColor(CURSESPP_BANNER);
    this->AddWindow(clientStatus);

    this->serverStatus = std::make_shared<TextLabel>();
    this->serverStatus->SetText("server: stopped", text::AlignCenter);
    this->serverStatus->SetContentColor(CURSESPP_BANNER);
    this->AddWindow(serverStatus);

    this->label = std::make_shared<TextLabel>();
    this->label->SetText("hello, autom8", text::AlignCenter);
    this->AddWindow(label);

    client->state_changed.connect(this, &MainLayout::OnClientStateChanged);
    autom8::server::started.connect(this, &MainLayout::OnServerStateChanged);
    autom8::server::stopped.connect(this, &MainLayout::OnServerStateChanged);

    this->Update();
}

void MainLayout::ResizeToViewport() {
    this->MoveAndResize(0, 0, Screen::GetWidth(), Screen::GetHeight());
}

void MainLayout::OnLayout() {
    int cx = this->GetContentWidth();
    this->clientStatus->MoveAndResize(0, 0, (cx / 2) - 1, 1);
    this->serverStatus->MoveAndResize(cx / 2, 0, cx - (cx / 2), 1);
    this->label->MoveAndResize(0, this->GetContentHeight() / 2, cx, 1);
}

void MainLayout::ProcessMessage(IMessage& message){
    if (message.Type() == UPDATE_STATUS_MESSAGE) {
        this->Update();
    }
}

void MainLayout::Update() {
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
        default:
            break;
    }

    this->clientStatus->SetText(std::string("client: ") + str, cursespp::text::AlignCenter);
    this->clientStatus->SetContentColor(color);

    str = "stopped";
    color = CURSESPP_BANNER;
    if (autom8::server::is_running()) {
        str = "running";
        color = CURSESPP_FOOTER;
    }

    this->serverStatus->SetText(std::string("server: ") + str, cursespp::text::AlignCenter);
    this->serverStatus->SetContentColor(color);
}

void MainLayout::OnServerStateChanged() {
    this->Post(UPDATE_STATUS_MESSAGE);
}

void MainLayout::OnClientStateChanged(autom8::client::connection_state state, autom8::client::reason reason) {
    this->Post(UPDATE_STATUS_MESSAGE);
}