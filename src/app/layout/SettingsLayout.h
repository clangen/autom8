#pragma once

#include <autom8/net/client.hpp>

#include <cursespp/TextLabel.h>
#include <cursespp/LayoutBase.h>
#include <cursespp/ListWindow.h>
#include <cursespp/ITopLevelLayout.h>

#include <f8n/runtime/IMessage.h>
#include <f8n/sdk/ISchema.h>

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
            void OnDeviceRowActivated(cursespp::ListWindow* window, size_t index);
            void OnAboutConfigActivated(cursespp::TextLabel* label);
            void OnAddDeviceActivated(cursespp::TextLabel* label);
            void OnDeviceControllerActivated(cursespp::TextLabel* label);
            void OnConfigureControllerActivated(cursespp::TextLabel* label);

            void Reload();

            autom8::client_ptr client;
            std::shared_ptr<f8n::sdk::ISchema> schema;
            std::shared_ptr<DeviceModelAdapter> deviceModelAdapter;
            std::shared_ptr<cursespp::TextLabel> aboutConfig;
            std::shared_ptr<cursespp::TextLabel> deviceController;
            std::shared_ptr<cursespp::TextLabel> configureController;
            std::shared_ptr<cursespp::TextLabel> addDevice;
            std::shared_ptr<cursespp::ListWindow> deviceModelList;
    };

} }