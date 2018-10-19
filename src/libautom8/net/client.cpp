#include <autom8/constants.h>
#include <autom8/net/client.hpp>
#include <autom8/util/ssl_certificate.hpp>
#include <autom8/message/message_formatter.hpp>
#include <autom8/message/message_matcher.hpp>
#include <autom8/message/common_messages.hpp>

#include <f8n/debug/debug.h>

#include <ostream>

#include <json.hpp>

#include <boost/bind.hpp>
#include <base64/base64.h>
#include <boost/format.hpp>

using namespace autom8;
using namespace nlohmann;
using debug = f8n::debug;

typedef boost::format format;
static request_ptr ping_(messages::requests::ping());
static response_ptr pong_(messages::responses::pong());
static response_ptr authenticate_(messages::responses::authenticated());
static const std::string TAG = "client";

client::client()
: hostname_("")
, port_(0)
, state_(state_disconnected)
, reason_(none)
, disconnect_reason_(client::unknown) {
}

client::client(const std::string& hostname, unsigned short port)
: hostname_(hostname)
, port_(port)
, state_(state_disconnected)
, reason_(none)
, disconnect_reason_(client::unknown)
{
}

client::~client() {
    disconnect();
}

void client::connect(
    const std::string& hostname,
    unsigned short port,
    const std::string& password)
{
    hostname_ = hostname;
    port_ = port;
    reconnect(password);
}

void client::reconnect(const std::string& password) {
    std::unique_lock<std::recursive_mutex> lock(state_lock_);

    disconnect("reconnecting");

    if (state_ != state_disconnected) {
        debug::warning(TAG, "connect() called but not disconnected");
        return;
    }

    password_ = password;

    connection_.reset(true);

    connection_.started(new std::thread(
        boost::bind(&client::io_service_thread_proc, this)));

    set_state(state_connecting);
}

void client::io_service_thread_proc() {
    connection::ssl_socket_ptr socket;
    connection::io_service_ptr io_service;

    {
        std::unique_lock<std::recursive_mutex> lock(state_lock_);
        socket = connection_.socket_;
        io_service = connection_.io_service_;
    }

    if (!socket || !io_service) {
        return;
    }

    tcp::resolver resolver(*io_service);
    tcp::resolver::query query(hostname_, std::to_string(port_));

    boost::system::error_code error;
    tcp::resolver::iterator begin = resolver.resolve(query, error);
    tcp::resolver::iterator end;

    if (error) {
        disconnect(connect_failed);
    }
    else {
        socket->set_verify_callback(
            boost::bind(&client::verify_certificate, this, _1, _2));

        auto handshakeCallback = [this]
            (const boost::system::error_code& error)
        {
            if (error) {
                disconnect(client::handshake_failed);
            }
            else {
                std::string pw;

                {
                    std::unique_lock<std::recursive_mutex> lock(state_lock_);
                    pw = password_;
                    password_.clear();
                }

                set_state(state_authenticating);
                send(messages::requests::authenticate(pw));
                async_read_next_message();
            }
        };

        auto connectCallback = [this, socket, handshakeCallback]
            (const boost::system::error_code& error,
            tcp::resolver::iterator endpoint_iterator)
        {
            if (error) {
                disconnect(client::connect_failed);
            }
            else {
                set_state(state_authenticating);

                debug::info(TAG, "handled_connect ok, starting handshake");

                socket->async_handshake(
                    boost::asio::ssl::stream_base::client,handshakeCallback);
            }
        };

        boost::asio::async_connect(
            socket->lowest_layer(), begin, end, connectCallback);

        io_service->run();
    }

    debug::info(TAG, "i/o service thread done");
}

bool client::verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx) {
#if 0
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // In this example we will simply print the certificate's subject name.
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    std::cout << "Verifying " << subject_name << "\n";

    return preverified;
#endif

    return true;
}

void client::disconnect(bool join) {
    disconnect(client::ok, join);
}

void client::disconnect(reason disconnect_reason, bool join) {
    debug::info(TAG, "attempting do disconnect: " + std::to_string(disconnect_reason));

    if (state_ == state_disconnected ||
        state_ == state_disconnecting)
    {
        debug::info(TAG, "disconnect called, but already disconnect[ed|ing]");
        return;
    }

    password_.clear();
    set_state(state_disconnecting, disconnect_reason);
    connection_.close(join);
    set_state(state_disconnected, disconnect_reason);
}

void client::set_state(connection_state state, reason reason) {
    std::unique_lock<std::recursive_mutex> lock(state_lock_);
    if (state != state_) {
        state_ = state;
        reason_ = reason;
        state_changed(state, reason);
    }
}

void client::async_read_next_message() {
    connection::ssl_socket_ptr socket;

    {
        std::unique_lock<std::recursive_mutex> lock(state_lock_);
        socket = connection_.socket_;
    }

    if (!socket) {
        return;
    }

    message_ptr m = std::make_shared<message>();

    auto nextMessageCallback = [this, m]
        (const boost::system::error_code& error, 
        std::size_t size)
    {
        if (error) {
            disconnect(client::read_failed);
        }
        else {
            if (!m->parse_message(size)) {
                debug::error(TAG, "message parse failed");
            }

            debug::info(TAG, "read message: " + m->name());

            if (m->type() == message::message_type_request) {
                on_recv(request::create(
                    "autom8://request/" + m->name(),
                    m->body()));
            }
            else if (m->type() == message::message_type_response) {
                response_ptr response = response::create(
                    "autom8://response/" + m->name(),
                    m->body(),
                    response::requester_only);

                if (state_ == state_authenticating) {
                    if (authenticate_->uri() == response->uri()) {
                        set_state(state_connected);
                    }
                    else {
                        set_state(state_disconnected, client::auth_failed);
                        return;
                    }
                }
                else {
                    on_recv(response);
                }
            }
            else {
                disconnect(client::bad_message);
                return;
            }

            async_read_next_message();
        }
    };

    boost::asio::async_read_until(
        *socket,
        m->read_buffer(),
        message_matcher(),
        nextMessageCallback);
}

client::connection_state client::state() {
    return state_;
}

void client::send(response_ptr r) {
    debug::info(TAG, "sending response: " + r->uri());
    send(message_formatter::create(r));
}

void client::send(request_ptr r) {
    debug::info(TAG, "sending request: " + r->uri());
    send(message_formatter::create(r));
}

void client::send(message_formatter_ptr f) {
    connection::ssl_socket_ptr socket;

    {
        std::unique_lock<std::recursive_mutex> lock(state_lock_);
        socket = connection_.socket_;
    }
    
    if (socket) {
        auto sendCallback = [this](
            const boost::system::error_code& error,
            std::size_t bytes_transferred)
        {
            if (error) {
                disconnect(client::write_failed);
            }
        };

        boost::asio::async_write(
            *socket,
            boost::asio::buffer(f->to_string()),
            sendCallback);
    }
}

void client::on_recv(response_ptr response) {
    debug::info(TAG, "on_recv(response): " + response->uri());
    recv_response(response); /* notify observers */
}

void client::on_recv(request_ptr request) {
    debug::info(TAG, "on_recv(request): " + request->uri());

    if (request->uri() == ping_->uri()) {
        send(pong_);
    }
    else {
        recv_request(request); /* notify observers */
    }
}
