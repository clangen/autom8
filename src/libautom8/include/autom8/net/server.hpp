#pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma warning(disable: 4503) // decorated name length exceeded, truncated
#endif

#include <autom8/net/session.hpp>
#include <autom8/message/request.hpp>
#include <autom8/util/signal_handler.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <boost/scoped_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/condition.hpp>

#include <sigslot/sigslot.h>
#include <set>

using boost::asio::ip::tcp;
using boost::system::error_code;

namespace autom8 {
    class server;
    typedef std::shared_ptr<server> server_ptr;
    typedef std::shared_ptr<boost::asio::deadline_timer> timer_ptr;

    class server: public signal_handler
                , public std::enable_shared_from_this<server> {
    private:
        typedef std::set<session_ptr> session_list;
        typedef boost::scoped_ptr<boost::thread> thread_ptr;

    public:
        virtual ~server();

        static sigslot::signal0<> started;
        static sigslot::signal0<> stopped;

        static bool start(int port = 7901);
        static bool stop();
        static bool is_running();
        static void send(session_ptr, response_ptr);
        static void send(session_ptr, request_ptr);
        static void send(response_ptr);
        static void send(request_ptr);

    private:
        server(int port);

        void start_instance();
        void stop_instance();

        void dispatch_response(session_ptr, response_ptr);
        void dispatch_request(session_ptr, request_ptr);
        void on_session_disconnected(session_ptr);
        void schedule_ping();
        void boostrap_new_session(session_ptr session);
        void handle_scheduled_ping(const error_code& error);
        void io_service_thread_proc();

        void start_accept();
        void handle_accept(const boost::system::error_code&, session_ptr);

        boost::asio::io_service io_service_;
        boost::asio::ssl::context ssl_context_;
        tcp::endpoint endpoint_;
        tcp::acceptor acceptor_;
        session_list session_list_;
        boost::mutex protect_session_list_mutex_;
        volatile bool stopped_;
        thread_ptr io_service_thread_;
        timer_ptr ping_timer_;
    };
}
