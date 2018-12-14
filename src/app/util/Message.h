#pragma once

#include <json.hpp>
#include <autom8/net/client.hpp>
#include <f8n/runtime/IMessage.h>
#include <f8n/runtime/IMessageTarget.h>

namespace autom8 { namespace app { namespace message {

    int CreateType();

    extern const int BROADCAST_SWITCH_TO_CLIENT_LAYOUT;
    extern const int BROADCAST_SWITCH_TO_CONSOLE_LAYOUT;
    extern const int BROADCAST_SWITCH_TO_SETTINGS_LAYOUT;

    class BaseMessage : public f8n::runtime::IMessage {
        protected:
            BaseMessage(
                f8n::runtime::IMessageTarget* target,
                int messageType,
                int64_t data1,
                int64_t data2)
            : target(target)
            , messageType(messageType)
            , data1(data1)
            , data2(data2)
            {
            }

        public:
            virtual ~BaseMessage() {
            }

            virtual f8n::runtime::IMessageTarget* Target() { return target; }
            virtual int Type() { return messageType; }
            virtual int64_t UserData1() { return data1; }
            virtual int64_t UserData2() { return data2; }

        private:
            f8n::runtime::IMessageTarget* target;
            int messageType;
            int64_t data1, data2;
    };

} } }