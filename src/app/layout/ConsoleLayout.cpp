#include "ConsoleLayout.h"
#include <f8n/i18n/Locale.h>
#include <cursespp/App.h>
#include <cursespp/Screen.h>
#include <cursespp/ToastOverlay.h>
#include <app/util/Device.h>
#include <app/util/Message.h>

using namespace autom8;
using namespace autom8::app;
using namespace cursespp;
using namespace f8n;
using namespace f8n::runtime;
using namespace f8n::sdk;

ConsoleLayout::ConsoleLayout(ConsoleLogger* logger)
: LayoutBase()
, logger(logger) {
    this->adapter = logger->Adapter();
    this->adapter->Changed.connect(this, &ConsoleLayout::OnAdapterChanged);

    this->listView = std::make_shared<ListWindow>(this->adapter);
    this->listView->SelectionChanged.connect(this, &ConsoleLayout::OnSelectionChanged);
    this->listView->EntryActivated.connect(this, &ConsoleLayout::OnItemActivated);
    this->listView->SetFrameTitle(_TSTR("console_list_title"));
    this->AddWindow(this->listView);
    this->listView->SetFocusOrder(0);
    this->listView->ScrollToBottom();
}

void ConsoleLayout::SetShortcutsWindow(cursespp::ShortcutsWindow* shortcuts) {
    this->shortcuts = shortcuts;

    if (shortcuts) {
        shortcuts->AddShortcut("~", _TSTR("shortcuts_console"));
        shortcuts->AddShortcut("d", _TSTR("shortcuts_devices"));
        shortcuts->AddShortcut("s", _TSTR("shortcuts_settings"));
        shortcuts->AddShortcut("^D", _TSTR("shortcuts_quit"));

        shortcuts->SetChangedCallback([this](std::string key) {
            if (key == "^D") {
                App::Instance().Quit();
            }
            else {
                this->KeyPress(key);
            }
        });

        shortcuts->SetActive("~");
    }
}

void ConsoleLayout::OnLayout() {
    LayoutBase::OnLayout();
    int cx = this->GetContentWidth();
    int cy = this->GetContentHeight();
    this->listView->MoveAndResize(0, 0, cx, cy);
}

void ConsoleLayout::OnAdapterChanged(SimpleScrollAdapter* adapter) {
    if (this->scrolledToBottom) {
        this->listView->ScrollToBottom();
    }
}

void ConsoleLayout::OnSelectionChanged(cursespp::ListWindow* window, size_t index, size_t prev) {
    auto count = this->logger->Adapter()->GetEntryCount();
    this->scrolledToBottom = index == (count - 1);
}

void ConsoleLayout::OnItemActivated(cursespp::ListWindow* window, size_t index) {
    ToastOverlay::Show(this->logger->Adapter()->StringAt(index), -1);
}

bool ConsoleLayout::KeyPress(const std::string& kn) {
    if (kn == "s") {
        Broadcast(message::BROADCAST_SWITCH_TO_SETTINGS_LAYOUT);
        return true;
    }
    if (kn == "d") {
        Broadcast(message::BROADCAST_SWITCH_TO_CLIENT_LAYOUT);
        return true;
    }
    return LayoutBase::KeyPress(kn);
}
