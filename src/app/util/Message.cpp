#include "Message.h"

namespace autom8 { namespace app { namespace message {

    int64_t CreateType() {
        static int64_t nextMessageType = 2048;
        return ++nextMessageType;
    }

}}}