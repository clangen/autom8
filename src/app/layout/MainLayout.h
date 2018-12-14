#pragma once

#include <autom8/net/client.hpp>
#include <autom8/net/server.hpp>

#include <cursespp/AppLayout.h>

#include <f8n/runtime/IMessage.h>

#include <app/layout/ClientLayout.h>
#include <app/layout/ConsoleLayout.h>
#include <app/layout/SettingsLayout.h>
#include <app/util/ConsoleLogger.h>

namespace autom8 { namespace app {

    class MainLayout: public cursespp::AppLayout {
        public:
            MainLayout(
                cursespp::App& app,
                ConsoleLogger* logger,
                autom8::client_ptr client);

            virtual ~MainLayout();
            virtual void OnLayout() override;
            virtual void ProcessMessage(f8n::runtime::IMessage& message) override;

        private:
            void UpdateStatus();

            void OnServerStateChanged();
            void OnClientStateChanged(
                autom8::client::connection_state state,
                autom8::client::reason reason);

            autom8::client_ptr client;
            std::shared_ptr<autom8::app::ClientLayout> clientLayout;
            std::shared_ptr<autom8::app::ConsoleLayout> consoleLayout;
            std::shared_ptr<autom8::app::SettingsLayout> settingsLayout;
            std::shared_ptr<cursespp::TextLabel> clientStatus;
            std::shared_ptr<cursespp::TextLabel> serverStatus;
    };

} }