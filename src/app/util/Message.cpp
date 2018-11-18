#include "Message.h"

namespace autom8 { namespace app { namespace message {

    int CreateType() {
        static int nextMessageType = 2048;
        return ++nextMessageType;
    }

    const int BROADCAST_SWITCH_TO_CLIENT_LAYOUT = CreateType();
    const int BROADCAST_SWITCH_TO_SETTINGS_LAYOUT = CreateType();

}}}