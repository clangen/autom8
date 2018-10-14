#pragma once

#include <autom8/net/client.hpp>
#include <autom8/net/server.hpp>

#include <cursespp/LayoutBase.h>
#include <cursespp/Screen.h>
#include <cursespp/TextLabel.h>
#include <cursespp/LayoutBase.h>
#include <cursespp/IViewRoot.h>

#include <f8n/runtime/IMessage.h>

namespace autom8 { namespace app {

    class MainLayout:
        public cursespp::LayoutBase,
        public cursespp::IViewRoot,
        public sigslot::has_slots<>
    {
        public:
            MainLayout(autom8::client_ptr client);

            virtual void ResizeToViewport() override;
            virtual void OnLayout() override;

            virtual void ProcessMessage(
                f8n::runtime::IMessage& message) override;

        private:
            void Update();
            void OnServerStateChanged();

            void OnClientStateChanged(
                autom8::client::connection_state state,
                autom8::client::reason reason);

            autom8::client_ptr client;
            std::shared_ptr<cursespp::TextLabel> label;
            std::shared_ptr<cursespp::TextLabel> clientStatus;
            std::shared_ptr<cursespp::TextLabel> serverStatus;
    };

} }