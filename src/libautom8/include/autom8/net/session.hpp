#ifndef __C_AUTOM8_SESSION_HPP__
#define __C_AUTOM8_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma warning(disable: 4503) // decorated name length exceeded, truncated
#endif

#include <autom8/message/message.hpp>
#include <autom8/message/message_formatter.hpp>
#include <autom8/message/response.hpp>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <sigslot/sigslot.h>
#include <thread>
#include <mutex>

using asio::ip::tcp;

namespace autom8 {
    typedef asio::ssl::stream<asio::ip::tcp::socket> ssl_socket;

    class session : public std::enable_shared_from_this<session> {
    public:
        using session_ptr = std::shared_ptr<session>;
        using thread_ptr = std::unique_ptr<std::thread>;
        using disconnect_signal_type = sigslot::signal1<session_ptr>;

    public:
        session(
            asio::io_service& io_service,
            asio::ssl::context& context);

        virtual ~session();

        void start();
        void disconnect(const std::string& reason = "");

        void enqueue_write(message_formatter_ptr formatter);

        ssl_socket& socket();
        std::string ip_address() const;
        bool is_authenticated() const;

        disconnect_signal_type& disconnect_signal();

    private:
        static bool handle_incoming_message(
            session_ptr session, message_ptr message);

        static bool handle_authentication(
            session_ptr session, message_ptr message);

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
