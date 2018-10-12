#ifndef __C_AUTOM8_SESSION_HPP__
#define __C_AUTOM8_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma warning(disable: 4503) // decorated name length exceeded, truncated
#endif

#include <autom8/message/message.hpp>
#include <autom8/message/message_formatter.hpp>
#include <autom8/message/message_queue.hpp>
#include <autom8/message/response.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <sigslot/sigslot.h>
#include <thread>

using boost::asio::ip::tcp;
using boost::system::error_code;

namespace autom8 {
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

    class session : public std::enable_shared_from_this<session> {
    public:
        using session_ptr = std::shared_ptr<session>;
        using thread_ptr = std::unique_ptr<std::thread>;

        typedef sigslot::signal1<session_ptr> disconnect_signal_type;
        typedef sigslot::signal1<int> session_destroyed_signal_type;

    public:
        session(boost::asio::io_service& io_service, boost::asio::ssl::context& context);
        virtual ~session();

        ssl_socket& socket();
        void start();
        std::string ip_address() const;
        bool is_authenticated() const;
        void enqueue_write(message_formatter_ptr formatter);
        disconnect_signal_type& disconnect_signal();

        void disconnect(const std::string& reason = "");
        void join();

    private:
        static bool handle_incoming_message(session_ptr session, message_ptr message);
        static bool handle_authentication(session_ptr session, message_ptr message);

        void on_disconnected();
        void read_thread_proc();
        void write_thread_proc();

        void async_read_next_message();

        void handle_next_read_message(
            message_ptr next_read,
            const boost::system::error_code& error,
            std::size_t size);

    private:
        ssl_socket socket_;
        volatile bool is_authenticated_;
        volatile bool is_disconnected_;
        std::string ip_address_;
        std::mutex write_lock_;
        thread_ptr write_thread_;
        disconnect_signal_type disconnect_signal_;
        message_queue_ptr write_queue_;
    };
}

#endif