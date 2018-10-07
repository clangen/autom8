#ifndef __C_AUTOM8_REQUEST_HANDLER_FACTORY_HPP__
#define __C_AUTOM8_REQUEST_HANDLER_FACTORY_HPP__

#include <autom8/message/request_handler.hpp>
#include <boost/thread.hpp>
#include <memory>

namespace autom8 {
    class session;

    class request_handler_factory;
    typedef std::shared_ptr<request_handler_factory> request_handler_factory_ptr;

    class request_handler_factory {
    private:
        typedef std::vector<request_handler_ptr> request_handler_list;
        request_handler_factory(); // singleton

    public:
        static request_handler_factory_ptr instance();
        bool handle_request(std::shared_ptr<session>, message_ptr);
        void register_handler(request_handler_ptr);

    private:
        boost::mutex protect_handler_list_mutex_;
        request_handler_list request_handlers_;
    };
}

#endif