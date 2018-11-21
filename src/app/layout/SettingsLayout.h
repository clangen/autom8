#pragma once

#include <autom8/net/client.hpp>

#include <cursespp/TextLabel.h>
#include <cursespp/LayoutBase.h>
#include <cursespp/ListWindow.h>
#include <cursespp/ITopLevelLayout.h>

#include <f8n/runtime/IMessage.h>

#include <app/adapter/DeviceModelAdapter.h>

namespace autom8 { namespace app {

    class SettingsLayout:
        public cursespp::LayoutBase,
        public cursespp::ITopLevelLayout,
        public sigslot::has_slots<>
    {
        public:
            SettingsLayout(autom8::client_ptr client);

            virtual void OnLayout() override;
            virtual void ProcessMessage(f8n::runtime::IMessage& message) override;
            virtual bool KeyPress(const std::string& kn) override;
            virtual void SetShortcutsWindow(cursespp::ShortcutsWindow* w) override;

        private:
            void OnAboutConfigActivated(cursespp::TextLabel* label);

            autom8::client_ptr client;
            std::shared_ptr<DeviceModelAdapter> deviceModelAdapter;
            std::shared_ptr<cursespp::TextLabel> aboutConfig;
            std::shared_ptr<cursespp::ListWindow> deviceModelList;
    };

} }