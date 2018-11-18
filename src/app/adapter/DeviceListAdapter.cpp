#include "DeviceListAdapter.h"

#include <cursespp/SingleLineEntry.h>
#include <cursespp/ScrollableWindow.h>
#include <cursespp/Colors.h>
#include <cursespp/Text.h>
#include <autom8/message/requests/get_device_list.hpp>
#include <f8n/debug/debug.h>
#include <f8n/runtime/Message.h>
#include <app/util/Message.h>
#include <algorithm>

using namespace cursespp;
using namespace f8n;
using namespace f8n::runtime;
using namespace autom8;
using namespace autom8::app;

static const int MESSAGE_DEVICE_LIST_CHANGED = app::message::CreateType();

DeviceListAdapter::DeviceListAdapter(
    autom8::client_ptr client,
    IMessageQueue& messageQueue,
    std::function<void()> changedCallback)
: client(client)
, messageQueue(messageQueue)
, changedCallback(changedCallback) {
    client->recv_response.connect(this, &DeviceListAdapter::OnClientResponse);

    client->state_changed.connect(this, &DeviceListAdapter::OnClientStateChanged);

    if (client->state() == client::state_connected) {
        this->Requery();
    }
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

void DeviceListAdapter::Requery() {
    client->send(autom8::get_device_list::request());
}

void DeviceListAdapter::NotifyChanged() {
    this->messageQueue.Post(Message::Create(this, MESSAGE_DEVICE_LIST_CHANGED));
}

const nlohmann::json DeviceListAdapter::At(const size_t index) {
    std::unique_lock<decltype(this->dataMutex)> lock(this->dataMutex);

    return this->data.size() > index
        ? this->data.at(index) : nlohmann::json::object();
}

static std::string formatRow(size_t width, const nlohmann::json& device) {
    static const int addressWidth = 3;
    static const int statusWidth = 3;

    const int labelWidth = std::max(0, (int) width - addressWidth - statusWidth - 6);
    const autom8::device_status status = device.value("status", autom8::device_status_unknown);

    const std::string addressStr = device.value("address", "??");
    
    std::string statusStr = "off";
    if (status == autom8::device_status_on) {
        statusStr = "on";
    }

    std::string label = device.value("label", "unknown device");
    label = text::Align(label, text::AlignLeft, labelWidth);

    return " " + addressStr + "  " +  label + "  " + statusStr + " ";
}

IScrollAdapter::EntryPtr DeviceListAdapter::GetEntry(ScrollableWindow* window, size_t index) {
    std::string label;

    bool selected = index == window->GetScrollPosition().logicalIndex;

    {
        std::unique_lock<decltype(this->dataMutex)> lock(this->dataMutex);
        label = formatRow(window->GetContentWidth(), this->data.at(index));
    }

    auto entry = std::make_shared<SingleLineEntry>(label);

    int64_t color = Color(selected
        ? Color::ListItemHighlighted : Color::Default);

    entry->SetAttrs(color);

    return entry;
}

void DeviceListAdapter::OnClientStateChanged(
    client::connection_state state, client::reason)
{
    if (state == client::state_connected) {
        this->Requery();
    }
    else if (state == client::state_disconnected) {
        this->data = nlohmann::json::object();
        this->NotifyChanged();
    }
}

void DeviceListAdapter::OnClientResponse(autom8::response_ptr response) {
    if (response->uri() == "autom8://response/get_device_list") {
        debug::info("DeviceListAdapter", "got device list response");
        std::unique_lock<decltype(this->dataMutex)> lock(this->dataMutex);
        this->data = response->body()->value("devices", nlohmann::json::array());
        this->NotifyChanged();
    }
    else if (response->uri() == "autom8://response/device_status_updated") {
        debug::info("DeviceListAdapter", "got device updated response");
        std::unique_lock<decltype(this->dataMutex)> lock(this->dataMutex);
        auto body = response->body();
        for (size_t i = 0; i < this->data.size(); i++) {
            auto& device = this->data.at(i);
            auto address = body->value("address", "");
            if (device.value("address", "") == address) {
                device_status updatedStatus = body->value("status", autom8::device_status_unknown);
                device["status"] = updatedStatus;
                this->NotifyChanged();
                break;
            }
        }
    }
}
