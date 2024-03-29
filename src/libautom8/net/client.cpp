#include <autom8/constants.h>
#include <autom8/net/client.hpp>
#include <autom8/util/ssl_certificate.hpp>
#include <autom8/message/message_formatter.hpp>
#include <autom8/message/message_matcher.hpp>
#include <autom8/message/common_messages.hpp>
#include <autom8/util/utility.hpp>

#include <f8n/debug/debug.h>
#include <f8n/str/util.h>

#include <ostream>
#include <functional>

#include <json.hpp>

#include <base64/base64.h>


using namespace autom8;
using namespace nlohmann;
using debug = f8n::debug;

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
    password_ = utility::sha256(password);
    reconnect();
}

void client::reconnect() {
    std::unique_lock<decltype(state_lock_)> lock(state_lock_);

    if (state_ != state_disconnected) {
        debug::warning(TAG, "connect() called but not disconnected");
        return;
    }

    connection_.reset();

    set_state(state_connecting);

    connection_.started(new std::thread(
        std::bind(&client::io_service_thread_proc, this)));
}

void client::io_service_thread_proc() {
    asio::ip::tcp::resolver resolver(*connection_.io_service_);
    asio::ip::tcp::resolver::query query(hostname_, std::to_string(port_));

    std::error_code error;
    asio::ip::tcp::resolver::iterator begin = resolver.resolve(query, error);
    asio::ip::tcp::resolver::iterator end;

    if (error) {
        disconnect(connect_failed);
    }
    else {
        connection_.socket_->set_verify_callback(
            std::bind(
                &client::verify_certificate,
                this,
                std::placeholders::_1,
                std::placeholders::_2));

        asio::async_connect(
            connection_.socket_->lowest_layer(),
            begin,
            end,
            std::bind(
                &client::handle_connect,
                this,
                std::placeholders::_1,
                std::placeholders::_2));

        connection_.io_service_->run();
    }

    debug::info(TAG, "i/o service thread done");
}

bool client::verify_certificate(bool preverified, asio::ssl::verify_context& ctx) {
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
    disconnect(client::user, join);
}

void client::disconnect(reason disconnect_reason, bool join) {
    debug::info(TAG, "attempting do disconnect: " + std::to_string(disconnect_reason));

    /* return early if disconnected/ing */
    {
        std::unique_lock<decltype(state_lock_)> lock(state_lock_);

        if (state_ == state_disconnected ||
            state_ == state_disconnecting)
        {
            debug::info(TAG, "disconnect called, but already disconnect[ed|ing]");
            return;
        }
    }

    set_state(state_disconnecting, disconnect_reason);

    connection_.close(join);

    set_state(state_disconnected, disconnect_reason);
}

void client::set_state(connection_state state, reason reason) {
    std::unique_lock<decltype(state_lock_)> lock(state_lock_);
    if (state != state_) {
        state_ = state;
        reason_ = reason;
        state_changed(state, reason);
    }
}

void client::handle_connect(
    const std::error_code& error,
    asio::ip::tcp::resolver::iterator endpoint_iterator)
{
    if (error) {
        disconnect(client::connect_failed);
    }
    else {
        set_state(state_authenticating);

        debug::info(TAG, "handled_connect ok, starting handshake");

        connection_.socket_->async_handshake(
            asio::ssl::stream_base::client,
            std::bind(
                &client::handle_handshake,
                this,
                std::placeholders::_1));
    }
}

void client::handle_handshake(const std::error_code& error) {
    if (error) {
        disconnect(client::handshake_failed);
    }
    else {
        set_state(state_authenticating);
        send(messages::requests::authenticate(password_));
        async_read_next_message();
    }
}

void client::schedule_ping() {
    std::unique_lock<decltype(state_lock_)> lock(state_lock_);
    if (state_ == state_connected) {
        if (connection_.ping_timer_) {
            connection_.ping_timer_->cancel();
        }

        connection_.ping_timer_ = connection::timer_ptr(
            new asio::high_resolution_timer(*connection_.io_service_));

        connection_.ping_timer_->expires_from_now(std::chrono::seconds(5));

        connection_.ping_timer_->async_wait(
            [this](const std::error_code& error) {
                if (!error) {
                    this->send(ping_);
                    this->schedule_ping();
                }
            });
    }
}

void client::async_read_next_message() {
    message_ptr m(new message());

    asio::async_read_until(
        *connection_.socket_,
        m->read_buffer(),
        message_matcher(),
        [this, message = m](const std::error_code& error, std::size_t size) {
            if (error || !size) {
                disconnect(client::read_failed);
            }
            else {
                if (!message->parse_message(size)) {
                    debug::error(TAG, "message parse failed");
                }

                if (message->name() != "ping" && message->name() != "pong") {
                    debug::info(TAG, "read message: " + message->name() + " " + message->body().dump());
                }

                if (message->type() == message::message_type_request) {
                    on_recv(request::create(
                        "autom8://request/" + message->name(),
                        message->body()));
                }
                else if (message->type() == message::message_type_response) {
                    /*
                     * convert the raw message to a response
                     */
                    response_ptr response = response::create(
                        "autom8://response/" + message->name(),
                        message->body(),
                        response::requester_only);

                    /*
                     * if we're not authenticated, the first response we should receive
                     * should be an authentication success. if it's not, then something
                     * fishy is going on... bail.
                     */
                    if (state_ == state_authenticating) {
                        if (authenticate_->uri() == response->uri()) {
                            set_state(state_connected);
                            schedule_ping();
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
        });
}

client::connection_state client::state() {
    return state_;
}

void client::send(response_ptr r) {
    if (r->uri() != "autom8://response/pong") {
        debug::info(TAG, "sending response: " + r->uri() + " " + r->body()->dump());
    }

    send(message_formatter::create(r));
}

void client::send(request_ptr r) {
    if (r->uri() != "autom8://request/ping") {
        if (r->uri() == "autom8://request/authenticate") {
            debug::info(TAG, "sending request: " + r->uri() + " [payload redacted]");
        }
        else {
            debug::info(TAG, "sending request: " + r->uri() + " " + r->body()->dump());
        }
    }

    send(message_formatter::create(r));
}

void client::send(message_formatter_ptr f) {
    std::unique_lock<decltype(state_lock_)> lock(state_lock_);

    if (state_ == state_connected ||
        state_ == state_connecting ||
        state_ == state_authenticating)
    {
        asio::async_write(
            *connection_.socket_,
            asio::buffer(f->to_string()),
            [this](const std::error_code& error, std::size_t size) {
                if (error || size == 0) {
                    this->disconnect(client::write_failed);
                }
            });
    }
}

void client::on_recv(response_ptr response) {
    recv_response(response); /* notify observers */
}

void client::on_recv(request_ptr request) {
    if (request->uri() == ping_->uri()) {
        send(pong_);
    }
    else {
        recv_request(request); /* notify observers */
    }
}
