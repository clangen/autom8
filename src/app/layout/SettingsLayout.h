#pragma once

#include <autom8/net/client.hpp>

#include <cursespp/TextLabel.h>
#include <cursespp/LayoutBase.h>
#include <cursespp/ListWindow.h>
#include <cursespp/ITopLevelLayout.h>

#include <f8n/runtime/IMessage.h>

namespace autom8 { namespace app {

    class SettingsLayout: 
        public cursespp::LayoutBase,
        public cursespp::ITopLevelLayout,
        public sigslot::has_slots<> 
    {
        public:
            SettingsLayout();

            virtual void OnLayout() override;
            virtual void ProcessMessage(f8n::runtime::IMessage& message) override;
            virtual bool KeyPress(const std::string& kn) override;
            virtual void SetShortcutsWindow(cursespp::ShortcutsWindow* w) override;

        private:
    };

} }