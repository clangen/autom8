#pragma once

#include <autom8/net/client.hpp>
#include <autom8/net/server.hpp>

#include <cursespp/LayoutBase.h>
#include <cursespp/ListWindow.h>
#include <cursespp/ITopLevelLayout.h>

#include <f8n/runtime/IMessage.h>

#include <app/util/ConsoleLogger.h>

namespace autom8 { namespace app {

    class ConsoleLayout:
        public cursespp::LayoutBase,
        public cursespp::ITopLevelLayout,
        public sigslot::has_slots<>
    {
        public:
            ConsoleLayout(ConsoleLogger* logger);

            virtual void OnLayout() override;
            virtual bool KeyPress(const std::string& kn) override;
            virtual void SetShortcutsWindow(cursespp::ShortcutsWindow* w) override;

        private:
            void OnAdapterChanged(cursespp::SimpleScrollAdapter* adapter);
            void OnSelectionChanged(cursespp::ListWindow* window, size_t index, size_t prev);
            void OnItemActivated(cursespp::ListWindow* window, size_t index);

            ConsoleLogger* logger;
            ConsoleLogger::AdapterPtr adapter;
            std::shared_ptr<cursespp::ListWindow> listView;
            cursespp::ShortcutsWindow* shortcuts;
            bool scrolledToBottom { true };
    };

} }