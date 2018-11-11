#include <json.hpp>

#include <autom8/message/request_handler.hpp>
#include <autom8/message/request.hpp>
#include <autom8/net/session.hpp>

namespace autom8 {
    class get_device_list: public request_handler {
    public:
        static request_handler_ptr create();
        static request_ptr request();
        virtual bool can_handle(session_ptr, message_ptr);
        virtual void operator()(session_ptr, message_ptr);
    private:
        get_device_list();
    };
}
