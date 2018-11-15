#pragma once

#include <autom8/net/client.hpp>
#include <cursespp/ScrollAdapterBase.h>
#include <f8n/runtime/IMessageQueue.h>
#include <f8n/runtime/IMessageTarget.h>
#include <functional>
#include <mutex>

namespace autom8 { namespace app {
    class DeviceListAdapter:
            public sigslot::has_slots<>,
            public f8n::runtime::IMessageTarget,
            public cursespp::ScrollAdapterBase
    {
        public:
            DeviceListAdapter(
                autom8::client_ptr client,
                f8n::runtime::IMessageQueue& messageQueue,
                std::function<void()> changedCallback);

            virtual ~DeviceListAdapter();

            virtual void ProcessMessage(f8n::runtime::IMessage &message) override;

            virtual size_t GetEntryCount() override;

            virtual cursespp::IScrollAdapter::EntryPtr GetEntry(
                cursespp::ScrollableWindow* window, size_t index) override;

            const nlohmann::json At(const size_t index);

        private:
            void OnClientResponse(autom8::response_ptr);

            void OnClientStateChanged(
                autom8::client::connection_state,
                autom8::client::reason);

            void Requery();

            std::mutex dataMutex;
            nlohmann::json data;
            autom8::client_ptr client;
            std::function<void()> changedCallback;
            f8n::runtime::IMessageQueue& messageQueue;
    };
} }