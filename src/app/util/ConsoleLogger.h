#pragma once

#include <autom8/net/client.hpp>
#include <f8n/debug/debug.h>
#include <cursespp/SimpleScrollAdapter.h>
#include <sigslot/sigslot.h>
#include <f8n/runtime/IMessageQueue.h>

namespace autom8 { namespace app {

    class ConsoleLogger:
        public f8n::debug::IBackend,
        public f8n::runtime::IMessageTarget,
        public sigslot::has_slots<>
    {
        public:
            using AdapterPtr = std::shared_ptr<cursespp::SimpleScrollAdapter>;

            ConsoleLogger(f8n::runtime::IMessageQueue& messageQueue);
            ConsoleLogger(const ConsoleLogger& other) = delete;
            ConsoleLogger& operator=(const ConsoleLogger& other) = delete;

            virtual void verbose(const std::string& tag, const std::string& string) override;
            virtual void info(const std::string& tag, const std::string& string) override;
            virtual void warning(const std::string& tag, const std::string& string) override;
            virtual void error(const std::string& tag, const std::string& string) override;

            virtual void ProcessMessage(f8n::runtime::IMessage &message) override;

            AdapterPtr Adapter();

        private:
            void FormatAndDispatch(
                const std::string& tag,
                const std::string& level,
                const std::string& str);

            AdapterPtr adapter;
            f8n::runtime::IMessageQueue& messageQueue;
    };

} }