#pragma once

#include <autom8/net/client.hpp>
#include <autom8/net/server.hpp>

#include <cursespp/LayoutBase.h>
#include <cursespp/Screen.h>
#include <cursespp/TextLabel.h>
#include <cursespp/LayoutBase.h>
#include <cursespp/ListWindow.h>

#include <f8n/runtime/IMessage.h>

#include <app/adapter/DeviceListAdapter.h>

namespace autom8 { namespace app {

    class MainLayout:
        public cursespp::LayoutBase,
        public sigslot::has_slots<>
    {
        public:
            MainLayout(autom8::client_ptr client);

            virtual void OnLayout() override;

            virtual void ProcessMessage(
                f8n::runtime::IMessage& message) override;

        private:
            void Update();
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