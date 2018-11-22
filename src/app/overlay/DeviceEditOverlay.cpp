#include "DeviceEditOverlay.h"
#include <cursespp/App.h>
#include <cursespp/Colors.h>
#include <cursespp/DialogOverlay.h>
#include <cursespp/Screen.h>
#include <f8n/utf/str.h>
#include <f8n/utf/conv.h>
#include <f8n/i18n/Locale.h>
#include <algorithm>
#include <locale>

using namespace f8n;
using namespace f8n::utf;
using namespace autom8;
using namespace autom8::app;
using namespace cursespp;

#define OVERLAY_HEIGHT 10
#define VERTICAL_PADDING 2
#define RIGHT(x) (x->GetX() + x->GetWidth())
#define TEXT_WIDTH(x) ((int) u8cols(x->GetText()))

static std::string groupsToString(device_ptr device) {
    std::string result = "";
    for (auto s : device->groups()) {
        if (result.size()) {
            result += ", ";
        }
        result += s;
    }
    return result;
}

static std::vector<std::string> stringToGroups(const std::string& str) {
    std::vector<std::string> result;
    // TODO
    return result;
}

static std::string typeToString(device_type type) {
    std::string localizedType = _TSTR("device_type_generic");
    switch (type) {
        case device_type_lamp:
            localizedType =  _TSTR("device_type_lamp");
            break;
        case device_type_appliance:
            localizedType = _TSTR("device_type_appliance");
            break;
        case device_type_security_sensor:
            localizedType = _TSTR("device_type_sensor");
            break;
    }
    return str::format(_TSTR("device_field_type"), localizedType.c_str());
}

static std::string typeToString(device_ptr device) {
    return typeToString(device->type());
}

static bool deviceWithAddressExists(
    const device_model& model,
    const std::string& address,
    database_id editingId = -1)
{
    for (auto d : model.all_devices()) {
        if (d->address() == address && (editingId == -1 || editingId != d->id())) {
            return true;
        }
    }
    return false;
}

static void showInvalidInputsDialog() {
    std::shared_ptr<DialogOverlay> dialog(new DialogOverlay());

    (*dialog)
        .SetTitle(_TSTR("settings_server_invalid_settings_title"))
        .SetMessage(_TSTR("settings_server_invalid_settings_message"))
        .AddButton("KEY_ENTER", "ENTER", _TSTR("button_ok"));

    App::Overlays().Push(dialog);
}

static void showDeviceWithAddressExistsDialog() {
    // TODO
}

static void showDeviceTypeOverlay(std::function<void(device_type)> callback) {
    // TODO
}

void DeviceEditOverlay::Create(device_system_ptr system) {
    App::Overlays().Push(std::make_shared<DeviceEditOverlay>(system, device_ptr()));
}

void DeviceEditOverlay::Edit(device_system_ptr system, device_ptr device) {
    App::Overlays().Push(std::make_shared<DeviceEditOverlay>(system, device));
}

void DeviceEditOverlay::Delete(
    device_system_ptr system,
    device_ptr device,
    std::function<void()> callback)
{
    std::shared_ptr<DialogOverlay> dialog(new DialogOverlay());

    (*dialog)
        .SetTitle(_TSTR("settings_delete_device_overlay_title"))
        .SetMessage(str::format(
            _TSTR("settings_delete_device_overlay_message"),
            device->address().c_str()))
        .AddButton("^[", "ESC", _TSTR("button_no"))
        .AddButton(
            "KEY_ENTER",
            "ENTER",
            _TSTR("button_yes"),
            [callback, system, device](const std::string& str) {
                if (system->model().remove(device->id()) && callback) {
                    callback();
                }
            });

    App::Overlays().Push(dialog);
}

DeviceEditOverlay::DeviceEditOverlay(device_system_ptr system, device_ptr device)
: OverlayBase()
, system(system)
, device(device) {
    int order = 0;

    this->deviceType = device ? device->type() : device_type_appliance;

    /* title */
    std::string title = _TSTR(device
        ? "device_edit_overlay_title"
        : "device_add_overlay_title");

    this->titleLabel = std::make_shared<TextLabel>(title, text::AlignCenter);
    this->titleLabel->SetBold(true);
    this->AddWindow(this->titleLabel);

    /* address */
    this->addressLabel = std::make_shared<TextLabel>(_TSTR("device_field_address"));
    this->AddWindow(this->addressLabel);

    this->addressInput = std::make_shared<TextInput>(TextInput::StyleLine);
    this->addressInput->SetFocusOrder(order++);
    this->AddWindow(this->addressInput);

    /* label */
    this->labelLabel = std::make_shared<TextLabel>(_TSTR("device_field_label"));
    this->AddWindow(this->labelLabel);

    this->labelInput = std::make_shared<TextInput>(TextInput::StyleLine);
    this->labelInput->SetFocusOrder(order++);
    this->AddWindow(this->labelInput);

    /* groups */
    this->groupsLabel = std::make_shared<TextLabel>(_TSTR("device_field_groups"));
    this->AddWindow(this->groupsLabel);

    this->groupsInput = std::make_shared<TextInput>(TextInput::StyleLine);
    this->groupsInput->SetFocusOrder(order++);
    this->AddWindow(this->groupsInput);

    /* type */
    this->typeLabel = std::make_shared<TextLabel>(typeToString(this->deviceType));
    this->typeLabel->SetFocusOrder(order++);
    this->AddWindow(this->typeLabel);

    /* shortcuts */
    this->shortcuts = std::make_shared<ShortcutsWindow>();
    this->shortcuts->SetAlignment(text::AlignRight);
    this->shortcuts->AddShortcut("ESC", _TSTR("button_cancel"));
    this->shortcuts->AddShortcut("M-s", _TSTR("button_save"));
    this->AddWindow(this->shortcuts);

    /* populate fields */
    if (device) {
        this->addressInput->SetText(device->address());
        this->labelInput->SetText(device->label());
        this->groupsInput->SetText(groupsToString(device));
    }

    /* styling */
    style(*this->titleLabel);
    style(*this->addressLabel);
    style(*this->addressInput);
    style(*this->labelLabel);
    style(*this->labelInput);
    style(*this->groupsLabel);
    style(*this->groupsInput);
    style(*this->typeLabel);
}

void DeviceEditOverlay::LayoutLabelAndInput(Label label, Input input, int y, int inputWidth) {
    int cx = this->GetContentWidth() - 2;
    int textWidth = TEXT_WIDTH(label);

    if (inputWidth == -1) {
        inputWidth = std::max(cx - textWidth, 0);
    }
    else {
        int maxWidth = std::max(cx - textWidth, 0);
        inputWidth = std::min(maxWidth, inputWidth);
    }

    label->MoveAndResize(1, y, textWidth, 1);
    input->MoveAndResize(1 + textWidth, y, inputWidth, 1);
}

void DeviceEditOverlay::Layout() {
    int width = (int)(0.8f * (float) Screen::GetWidth());
    int height = OVERLAY_HEIGHT;
    int y = VERTICAL_PADDING;
    int x = (Screen::GetWidth() / 2) - (width / 2);
    this->MoveAndResize(x, y, width, height);

    int cx = this->GetContentWidth() - 2;
    int cy = this->GetContentHeight();

    y = 0;
    this->titleLabel->MoveAndResize(1, 0, cx, 1);

    y +=2;
    this->LayoutLabelAndInput(this->addressLabel, this->addressInput, y++, 8);
    this->LayoutLabelAndInput(this->labelLabel, this->labelInput, y++);
    this->LayoutLabelAndInput(this->groupsLabel, this->groupsInput, y++);
    this->typeLabel->MoveAndResize(1, y++, cx, 1);

    this->shortcuts->MoveAndResize(1, cy - 1, cx, 1);
}

void DeviceEditOverlay::UpdateOrAddDevice() {
    std::locale locale;
    auto address = std::tolower(this->addressInput->GetText(), locale);
    auto label = this->labelInput->GetText();
    auto type = this->deviceType;
    std::vector<std::string> groups = stringToGroups(this->groupsInput->GetText());

    if (!address.size() || !label.size()) {
        showInvalidInputsDialog();
    }
    else if (this->device) {
        if (deviceWithAddressExists(this->system->model(), address, device->id())) {
            showDeviceWithAddressExistsDialog();
        }
        else {
            this->system->model().update(
                device->id(), type, address, label, groups);
        }
    }
    else {
        if (deviceWithAddressExists(this->system->model(), address)) {
            showDeviceWithAddressExistsDialog();
        }
        else {
            this->system->model().add(type, address, label, groups);
        }
    }
}

bool DeviceEditOverlay::KeyPress(const std::string& key) {
    if (key == "^[" || key == "ESC") { /* esc closes */
        this->Dismiss();
        return true;
    }
    else if (key == "M-s") {
        this->UpdateOrAddDevice();
        return true;
    }
    return OverlayBase::KeyPress(key);
}
