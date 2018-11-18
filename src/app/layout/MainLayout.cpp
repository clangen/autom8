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
    this->SetLayout(this->clientLayout);
    this->Post(REGISTER_FOR_BROADCASTS);
}

MainLayout::~MainLayout() {
    MessageQueue().UnregisterForBroadcasts(this);
}

void MainLayout::ProcessMessage(f8n::runtime::IMessage& message) {
    AppLayout::ProcessMessage(message);
    if (message.Type() == REGISTER_FOR_BROADCASTS) {
        MessageQueue().RegisterForBroadcasts(shared_from_this());
    }
}
