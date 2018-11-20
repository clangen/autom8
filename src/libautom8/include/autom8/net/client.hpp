#pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma warning(disable: 4503) // decorated name length exceeded, truncated
#endif

#include <autom8/message/request.hpp>
#include <autom8/message/response.hpp>
#include <autom8/message/message_formatter.hpp>
#include <autom8/util/signal_handler.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl.hpp>

#include <sigslot/sigslot.h>
#include <memory>
#include <set>
#include <memory>
#include <utility>
#include <future>

using boost::asio::ip::tcp;
using boost::system::error_code;

namespace autom8 {
    class client;
    typedef std::shared_ptr<client> client_ptr;
    typedef std::shared_ptr<boost::asio::deadline_timer> timer_ptr;

    class client: public signal_handler
                , public std::enable_shared_from_this<client> {
    public:
        enum connection_state {
            state_disconnected = 0,
            state_connecting,
            state_connected,
            state_authenticating,
            state_disconnecting,
        };

        enum reason {
            unknown = -1,
            none = 0,
            user = 1,
            connect_failed = 2,
            handshake_failed = 3,
            auth_failed = 4,
            bad_message = 5,
            read_failed = 6,
            write_failed = 7
        };

    public:
        client();
        client(const std::string& hostname, unsigned short port);

        virtual ~client();

        sigslot::signal2<connection_state, reason> state_changed;
        sigslot::signal1<request_ptr> recv_request;
        sigslot::signal1<response_ptr> recv_response;

        void connect(
            const std::string& hostname,
            unsigned short port,
            const std::string& password);

        std::string hostname() { return hostname_; }

        void reconnect();

        void disconnect(bool join = false);
        connection_state state();

        void send(response_ptr);
        void send(request_ptr);

    private:
        void handle_connect(
            const boost::system::error_code& error,
            tcp::resolver::iterator endpoint_iterator);

        void handle_handshake(
            const boost::system::error_code& error);

        bool verify_certificate(
            bool preverified,
            boost::asio::ssl::verify_context& ctx);

        void async_read_next_message();

        void set_state(connection_state state, reason reason = none);

        void on_recv(response_ptr);
        void on_recv(request_ptr);
        void io_service_thread_proc();
        void schedule_ping();

        void send(message_formatter_ptr);
        void disconnect(reason disconnect_reason, bool join = false);

    private:
        struct connection {
            using ssl_socket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
            using ssl_socket_ptr = std::unique_ptr<ssl_socket> ;
            using ssl_context =  boost::asio::ssl::context;
            using ssl_context_ptr = std::unique_ptr<ssl_context>;
            using io_service_ptr = std::unique_ptr <boost::asio::io_service>;
            using thread = std::thread;
            using thread_ptr = std::unique_ptr<thread>;
            using timer_ptr = std::unique_ptr<boost::asio::deadline_timer>;

            ssl_context_ptr ssl_context_;
            ssl_socket_ptr socket_;
            io_service_ptr io_service_;
            thread_ptr service_thread_;
            timer_ptr ping_timer_;

            connection() { reset(); }

            ~connection() { close(); }

            void close(bool join = false) {
                if (io_service_ || service_thread_ || socket_ || ssl_context_) {
                    auto thread = std::thread([ /* deferred disconnect */
                        io_service_ = std::move(this->io_service_),
                        service_thread_ = std::move(this->service_thread_),
                        socket_ = std::move(this->socket_),
                        ssl_context_ = std::move(this->ssl_context_),
                        ping_timer_ = std::move(this->ping_timer_)]
                    {
                        if (ping_timer_) { ping_timer_->cancel(); }
                        if (socket_) { socket_->lowest_layer().close(); }
                        if (io_service_) { io_service_->stop(); }
                        if (service_thread_) { service_thread_->join(); }
                    });

                    if (thread.joinable()) {
                        if (join) {
                            thread.join();
                        }
                        else {
                            thread.detach();
                        }
                    }
                }
            }

            void reset() {
                close();
                io_service_ = std::make_unique<boost::asio::io_service>();
                ssl_context_ = std::make_unique<ssl_context>(boost::asio::ssl::context::sslv23);
                ssl_context_->set_verify_mode(boost::asio::ssl::context::verify_peer);
                ssl_context_->set_options(boost::asio::ssl::context::default_workarounds);
                socket_ = std::make_unique<ssl_socket>(*io_service_, *ssl_context_);
            }

            void started(thread* t) {
                service_thread_.reset(t);
            }
        };

        connection connection_;
        std::string hostname_;
        unsigned short port_;
        std::string password_;
        connection_state state_;
        reason reason_;
        std::recursive_mutex state_lock_;
        reason disconnect_reason_;
    };
}
