#pragma once

#include <autom8/message/request_handler.hpp>
#include <boost/thread.hpp>
#include <memory>

namespace autom8 {
    class session;

    class request_handler_factory {
        public:
            using session_ptr = std::shared_ptr<session>;
            typedef std::shared_ptr<request_handler_factory> ptr;

        private:
            typedef std::vector<request_handler_ptr> request_handler_list;
            request_handler_factory(); /* singleton */

        public:
            static ptr instance();
            bool handle_request(session_ptr, message_ptr);
            void register_handler(request_handler_ptr);

        private:
            boost::mutex protect_handler_list_mutex_;
            request_handler_list request_handlers_;
    };
}
