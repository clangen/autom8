#include "DeviceModelAdapter.h"

#include <cursespp/SingleLineEntry.h>
#include <cursespp/ScrollableWindow.h>
#include <cursespp/Colors.h>
#include <cursespp/Text.h>
#include <autom8/message/requests/get_device_list.hpp>
#include <f8n/debug/debug.h>
#include <f8n/i18n/Locale.h>
#include <f8n/runtime/Message.h>
#include <app/util/Message.h>
#include <algorithm>

using namespace cursespp;
using namespace f8n;
using namespace f8n::runtime;
using namespace autom8;
using namespace autom8::app;

static const int MESSAGE_DEVICE_LIST_CHANGED = app::message::CreateType();

static int maxWidth(const std::vector<std::string>&& all) {
    size_t max = 0;
    for (auto s: all) {
        auto len = _TSTR(s).size();
        if (len > max) {
            max = len;
        }
    }
    return max;
}

static int maxAddress(const device_list& devices) {
    size_t max = 0;
    for (auto d : devices) {
        if (d->address().size() > max) {
            max = d->address().size();
        }
    }
    return max;
}

DeviceModelAdapter::DeviceModelAdapter(device_system_ptr system)
: system(system) {
    this->Requery();
}

DeviceModelAdapter::~DeviceModelAdapter() {

}

void DeviceModelAdapter::Requery() {
    this->devices = system->model().all_devices();
}

size_t DeviceModelAdapter::GetEntryCount() {
    return this->devices.size();
}

const device_ptr DeviceModelAdapter::At(const size_t index) {
    return this->devices.at(index);
}

static std::string formatRow(size_t width, const device_list& all, device_ptr device) {
    const int addressWidth = maxAddress(all);

    const int typeWidth = maxWidth({
        "device_type_appliance",
        "device_type_lamp",
        "device_type_sensor",
        "device_type_generic"
    });

    const int labelWidth = std::max(0, (int) width - addressWidth - typeWidth - 2);

    const std::string addressStr = device->address();

    std::string typeStr = _TSTR("device_type_generic");
    switch (device->type()) {
        case autom8::device_type_appliance:
            typeStr = _TSTR("device_type_appliance");
            break;
        case autom8::device_type_lamp:
            typeStr = _TSTR("device_type_lamp");
            break;
        case autom8::device_type_security_sensor:
            typeStr = _TSTR("device_type_sensor");
            break;
        default:
            break;
    }

    std::string label = text::Align(device->label(), text::AlignLeft, labelWidth);

    return " " + addressStr + "  " +  label + "  " + typeStr + " ";
}

IScrollAdapter::EntryPtr DeviceModelAdapter::GetEntry(ScrollableWindow* window, size_t index) {
    bool selected = index == window->GetScrollPosition().logicalIndex;

    std::string label = formatRow(
        window->GetContentWidth(),
        this->devices,
        this->devices.at(index));

    auto entry = std::make_shared<SingleLineEntry>(label);

    int64_t color = Color(selected
        ? Color::ListItemHighlighted : Color::Default);

    entry->SetAttrs(color);

    return entry;
}
