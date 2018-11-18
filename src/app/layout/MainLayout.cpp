#include "MainLayout.h"
#include <app/util/Device.h>
#include <cursespp/Colors.h>
#include <f8n/debug/debug.h>

using namespace autom8;
using namespace autom8::app;
using namespace cursespp;
using namespace f8n;
using namespace f8n::runtime;

static const int UPDATE_STATUS_MESSAGE = 1024;
static const int SCHEDULE_RECONNECT = 1025;

MainLayout::MainLayout(App& app, client_ptr client)
: AppLayout(app) {
    this->clientLayout = std::make_shared<ClientLayout>(client);
    this->SetLayout(this->clientLayout);
}
