#include "DeviceListAdapter.h"

#include <cursespp/SingleLineEntry.h>
#include <cursespp/ScrollableWindow.h>
#include <cursespp/Colors.h>
#include <f8n/debug/debug.h>
#include <f8n/runtime/Message.h>

using namespace cursespp;
using namespace f8n;
using namespace f8n::runtime;
using namespace autom8;
using namespace autom8::app;

static const int MESSAGE_DEVICE_LIST_CHANGED = 1024;

DeviceListAdapter::DeviceListAdapter(
    autom8::client_ptr client,
    IMessageQueue& messageQueue,
    std::function<void()> changedCallback)
: client(client)
, messageQueue(messageQueue)
, changedCallback(changedCallback) {
    client->recv_response.connect(this, &DeviceListAdapter::OnClientResponse);
}

DeviceListAdapter::~DeviceListAdapter() {

}

void DeviceListAdapter::ProcessMessage(IMessage &message) {
    if (message.Type() == MESSAGE_DEVICE_LIST_CHANGED) {
        this->changedCallback();
    }
}

size_t DeviceListAdapter::GetEntryCount() {
    std::unique_lock<decltype(this->dataMutex)> lock(this->dataMutex);
    return this->data.size();
}

IScrollAdapter::EntryPtr DeviceListAdapter::GetEntry(ScrollableWindow* window, size_t index) {
    std::string label = "unknown device";

    bool selected = index == window->GetScrollPosition().logicalIndex;

    {
        std::unique_lock<decltype(this->dataMutex)> lock(this->dataMutex);
        label = this->data.at(index).value("label", label);
    }

    auto entry = std::make_shared<SingleLineEntry>(label);

    int64_t color = Color(selected
        ? Color::ListItemHighlighted : Color::Default);

    entry->SetAttrs(color);

    return entry;
}

void DeviceListAdapter::OnClientResponse(autom8::response_ptr response) {
    if (response->uri() == "autom8://response/get_device_list") {
        std::unique_lock<decltype(this->dataMutex)> lock(this->dataMutex);
        this->data = response->body()->value("devices", nlohmann::json::array());
        this->messageQueue.Post(Message::Create(this, MESSAGE_DEVICE_LIST_CHANGED));
        debug::info("DeviceListAdapter", "got device list response");
    }
}
