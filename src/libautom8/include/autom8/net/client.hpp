#ifndef __C_AUTOM8_CLIENT_HPP__
#define __C_AUTOM8_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma warning(disable: 4503) // decorated name length exceeded, truncated
#endif

#include <autom8/message/request.hpp>
#include <autom8/message/response.hpp>
#include <autom8/message/message_formatter.hpp>
#include <autom8/util/signal_handler.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/condition.hpp>
#include <boost/asio/ssl.hpp>

#include <sigslot/sigslot.h>
#include <set>

using boost::asio::ip::tcp;
using boost::system::error_code;

namespace autom8 {
    class client;
    typedef boost::shared_ptr<client> client_ptr;
    typedef boost::shared_ptr<boost::asio::deadline_timer> timer_ptr;

    class client: public signal_handler
                , public boost::enable_shared_from_this<client> {
    private:
        typedef boost::scoped_ptr<boost::thread> thread_ptr;
        typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

    public:
        enum connection_state {
            state_disconnected = 0,
            state_connecting,
            state_connected,
            state_authenticating,
            state_disconnecting,
            state_dead
        };

        enum reason {
            unknown = -1,
            ok = 0,
            connect_failed = 1,
            handshake_failed = 2,
            auth_failed = 3,
            bad_message = 4,
            read_failed = 5,
            write_failed = 6
        };

    public:
        client(const std::string& hostname, const std::string& port);
        virtual ~client();

        sigslot::signal0<> connected;
        sigslot::signal1<reason> disconnected;
        sigslot::signal1<request_ptr> recv_request;
        sigslot::signal1<response_ptr> recv_response;

        void connect(const std::string& password);
        void disconnect();
        connection_state state();

        void send(response_ptr);
        void send(request_ptr);

    private:
        void handle_connect(
            const boost::system::error_code& error,
            tcp::resolver::iterator endpoint_iterator);

        void handle_handshake(
            const boost::system::error_code& error);

        void handle_next_read_message(
            message_ptr next_read,
            const boost::system::error_code& error,
            std::size_t size);

        void handle_post_send(
            message_formatter_ptr formatter,
            const boost::system::error_code& error,
            std::size_t size);

        bool verify_certificate(
            bool preverified,
            boost::asio::ssl::verify_context& ctx);

        void async_read_next_message();

        void on_recv(response_ptr);
        void on_recv(request_ptr);
        void io_service_thread_proc();

        void send(message_formatter_ptr);
        void disconnect(const std::string& reason);

        void set_disconnect_reason(reason r);

    private:
        std::string hostname_;
        std::string port_;
        std::string password_;
        boost::asio::io_service io_service_;
        boost::asio::ssl::context ssl_context_;
        ssl_socket socket_;
        connection_state state_;
        boost::mutex state_lock_;
        reason disconnect_reason_;
        thread_ptr io_service_thread_;
    };
}

#endif