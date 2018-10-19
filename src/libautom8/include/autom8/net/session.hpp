#ifndef __C_AUTOM8_SESSION_HPP__
#define __C_AUTOM8_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma warning(disable: 4503) // decorated name length exceeded, truncated
#endif

#include <autom8/message/message.hpp>
#include <autom8/message/message_formatter.hpp>
#include <autom8/message/response.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <sigslot/sigslot.h>
#include <thread>
#include <mutex>

using boost::asio::ip::tcp;
using boost::system::error_code;

namespace autom8 {
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

    class session : public std::enable_shared_from_this<session> {
    public:
        using session_ptr = std::shared_ptr<session>;
        using thread_ptr = std::unique_ptr<std::thread>;
        using disconnect_signal_type = sigslot::signal1<session_ptr>;

    public:
        session(
            boost::asio::io_service& io_service,
            boost::asio::ssl::context& context);

        virtual ~session();

        void start();
        void disconnect(const std::string& reason = "");

        void send(request_ptr request);
        void send(response_ptr response);

        ssl_socket& socket();
        std::string ip_address() const;
        bool is_authenticated() const;

        disconnect_signal_type& disconnect_signal();

    private:
        static bool handle_incoming_message(
            session_ptr session, message_ptr message);

        static bool handle_authentication(
            session_ptr session, message_ptr message);

        void enqueue_write(message_formatter_ptr formatter);
        void on_disconnected();
        void async_read_next_message();

    private:
        ssl_socket socket_;
        volatile bool is_authenticated_;
        volatile bool is_disconnected_;
        std::string ip_address_;
        disconnect_signal_type disconnect_signal_;
    };
}

#endif
