#pragma once

#include <autom8/net/client.hpp>
#include <autom8/net/server.hpp>

#include <cursespp/AppLayout.h>

#include <f8n/runtime/IMessage.h>

#include <app/layout/ClientLayout.h>

namespace autom8 { namespace app {

    class MainLayout: public cursespp::AppLayout {
        public:
            MainLayout(
                cursespp::App& app,
                autom8::client_ptr client);

            virtual ~MainLayout();

            virtual void ProcessMessage(f8n::runtime::IMessage& message) override;

        private:
            std::shared_ptr<autom8::app::ClientLayout> clientLayout;
    };

} }