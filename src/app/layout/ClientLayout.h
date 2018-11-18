#pragma once

#include <autom8/net/client.hpp>
#include <autom8/net/server.hpp>

#include <cursespp/TextLabel.h>
#include <cursespp/LayoutBase.h>
#include <cursespp/ListWindow.h>
#include <cursespp/ITopLevelLayout.h>

#include <f8n/runtime/IMessage.h>

#include <app/adapter/DeviceListAdapter.h>
#include <app/util/Device.h>

namespace autom8 { namespace app {

    class ClientLayout:
        public cursespp::LayoutBase,
        public cursespp::ITopLevelLayout,
        public sigslot::has_slots<>
    {
        public:
            ClientLayout(autom8::client_ptr client);

            virtual void OnLayout() override;
            virtual void ProcessMessage(f8n::runtime::IMessage& message) override;
            virtual bool KeyPress(const std::string& kn) override;
            virtual void SetShortcutsWindow(cursespp::ShortcutsWindow* w) override;

        private:
            void Update();

            void OnDeviceRowActivated(cursespp::ListWindow* w, size_t index);

            void OnServerStateChanged();

            void OnClientStateChanged(
                autom8::client::connection_state state,
                autom8::client::reason reason);

            std::shared_ptr<DeviceListAdapter> deviceListAdapter;
            autom8::client_ptr client;
            std::shared_ptr<cursespp::TextLabel> clientStatus;
            std::shared_ptr<cursespp::TextLabel> serverStatus;
            std::shared_ptr<cursespp::ListWindow> deviceList;
    };

} }