#include "Message.h"

namespace autom8 { namespace app { namespace message {

    int64_t CreateType() {
        static int64_t nextMessageType = 2048;
        return ++nextMessageType;
    }

    const int64_t BROADCAST_SWITCH_TO_CLIENT_LAYOUT = CreateType();
    const int64_t BROADCAST_SWITCH_TO_SETTINGS_LAYOUT = CreateType();

}}}