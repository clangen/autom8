#pragma once

#include <autom8/net/client.hpp>
#include <autom8/net/server.hpp>

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
            virtual bool KeyPress(const std::string& kn) override;
            virtual void SetShortcutsWindow(cursespp::ShortcutsWindow* w) override;

        private:
            void OnDeviceRowActivated(cursespp::ListWindow* window, size_t index);
            void OnAddDeviceActivated(cursespp::TextLabel* label);
            void OnConfigureControllerActivated(cursespp::TextLabel* label);
            void OnConfigureClientActivated(cursespp::TextLabel* label);
            void OnConfigureServerActivated(cursespp::TextLabel* label);

            void ReloadController();
            void ReloadClient();
            void ReloadServer();

            autom8::client_ptr client;
            std::shared_ptr<f8n::sdk::ISchema> schema;
            std::shared_ptr<DeviceModelAdapter> deviceModelAdapter;
            std::shared_ptr<cursespp::TextLabel> clientConfig;
            std::shared_ptr<cursespp::TextLabel> serverConfig;
            std::shared_ptr<cursespp::TextLabel> configureController;
            std::shared_ptr<cursespp::TextLabel> addDevice;
            std::shared_ptr<cursespp::ListWindow> deviceModelList;
    };

} }