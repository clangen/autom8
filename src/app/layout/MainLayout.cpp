#include "MainLayout.h"
#include <app/util/Message.h>
#include <cursespp/Colors.h>
#include <f8n/debug/debug.h>

using namespace autom8;
using namespace autom8::app;
using namespace cursespp;
using namespace f8n;
using namespace f8n::runtime;

static const int REGISTER_FOR_BROADCASTS = app::message::CreateType();

MainLayout::MainLayout(App& app, client_ptr client)
: AppLayout(app) {
    this->clientLayout = std::make_shared<ClientLayout>(client);
    this->settingsLayout = std::make_shared<SettingsLayout>(client);
    this->SetLayout(this->clientLayout);
    this->Post(REGISTER_FOR_BROADCASTS);
}

MainLayout::~MainLayout() {
    MessageQueue().UnregisterForBroadcasts(this);
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
    else {
        AppLayout::ProcessMessage(message);
    }
}
