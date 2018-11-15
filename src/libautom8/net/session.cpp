#include <autom8/constants.h>
#include <autom8/net/session.hpp>
#include <autom8/message/request_handler_factory.hpp>
#include <autom8/message/common_messages.hpp>
#include <autom8/net/server.hpp>
#include <autom8/util/utility.hpp>
#include <autom8/message/message_matcher.hpp>
#include <f8n/debug/debug.h>
#include <boost/format.hpp>
#include <atomic>

using namespace autom8;
using namespace nlohmann;
using debug = f8n::debug;

static std::atomic<int> instance_count_ { 0 };
static const std::string TAG = "session";

inline void print_instance_count() {
    debug::info(TAG, (boost::format("session instance count: %1%") % instance_count_).str());
}

inline void inc_instance_count() {
    ++instance_count_;
    print_instance_count();
}

inline void dec_instance_count() {
    --instance_count_;
    print_instance_count();
}

session::session(boost::asio::io_service& io_service, boost::asio::ssl::context& context)
: socket_(io_service, context)
, is_authenticated_(false)
, is_disconnected_(true) {
    inc_instance_count();
}

session::~session() {
    dec_instance_count();
}

session::disconnect_signal_type& session::disconnect_signal() {
    return disconnect_signal_;
}

ssl_socket& session::socket() {
    return socket_;
}

void session::start() {
    if (!is_disconnected_) {
        disconnect("session already started, but start() called. disconnecting now.");
        return;
    }

    try {
        socket_.handshake(boost::asio::ssl::stream_base::server);

        is_disconnected_ = false;
        ip_address_ = (socket_.lowest_layer().remote_endpoint().address().to_string());

        async_read_next_message();
    }
    catch (...) {
        disconnect("exception caught, session disconnecting");
    }
}

std::string session::ip_address() const {
    return ip_address_;
}

bool session::is_authenticated() const {
    return is_authenticated_;
}

void session::enqueue_write(message_formatter_ptr formatter) {
    /* we should never be asked to write before we auth. */
    if (!is_authenticated_) {
        debug::warning(TAG, "message received before authenticated. ignoring.");
        return;
    }

    boost::asio::async_write(
        socket_,
        boost::asio::buffer(formatter->to_string()),
        [this](const boost::system::error_code& error, std::size_t bytes) {
            if (error) {
                this->disconnect("message write failed");
            }
        });
}

bool session::handle_authentication(session_ptr session, message_ptr message) {
    if ((message->type() == message::message_type_request)) {
        if (message->name() == "authenticate") {
            std::string stored_password;
            utility::prefs().get("password", stored_password);

            auto document = message->body();
            std::string received_password = document["password"];
            session->is_authenticated_ = (stored_password == received_password);

            if (session->is_authenticated_) {
                server::send(session, messages::responses::authenticated());
                return true;
            }
            else {
                debug::error(TAG, "authenticate password mismatch");
            }
        }
        else {
            debug::error(TAG, "expected authenticate, but got: " + message->name());
        }
    }
    else {
        debug::error(TAG, "expected request, but got response");
    }

    message_formatter_ptr f =message_formatter::create(
        messages::responses::authenticate_failed());

    boost::system::error_code ec;
    write(session->socket_, boost::asio::buffer(f->to_string()), ec);
    session->disconnect("authenticate failed");

    return false;
}

bool session::handle_incoming_message(session_ptr session, message_ptr m) {
    switch (m->type()) {
        case message::message_type_request:
            {
                // handle the ping request here, so other handlers can't intercept it.
                if (m->name() == "ping") {
                    server::send(session, messages::responses::pong());
                    return true;
                }

                debug::info(TAG, "recv: autom8://request/" + m->name());
                request_handler_factory::ptr factory = request_handler_factory::instance();
                if (!factory->handle_request(session, m)) {
                    return false;
                }
            }
            return true;

        case message::message_type_response:
            {
                if (m->name() != "pong") { /* these pollute the the log */
                    debug::info(TAG, "recv: autom8://response/" + m->name());
                }
            }
            return true;

        default:
            return false;
    }
}

void session::on_disconnected() {
    disconnect_signal_(shared_from_this());
}

void session::disconnect(const std::string& reason) {
    if (!is_disconnected_) {
        if (reason.size()) {
            debug::info(TAG, reason);
        }

        is_disconnected_ = true;

        try {
            boost::system::error_code ec;
            socket_.lowest_layer().cancel(ec);
            socket_.lowest_layer().close();
        }
        catch(...) {
            debug::warning(TAG, "failed to close() socket");
        }
    }

    /* cache a shared pointer to ensure we don't get cleaned up
    before the thread has a chance to run */
    auto shared = shared_from_this();
    std::thread([shared] { shared->on_disconnected(); }).detach();
}

void session::async_read_next_message() {
    message_ptr m = message_ptr(new message());

    auto callback = [this, m](const boost::system::error_code& error, std::size_t bytes_read) {
        if (error) {
            this->disconnect("socket read() failed");
        }
        else if (bytes_read > 0) {
            if (!m->parse_message(bytes_read)) {
                this->disconnect("failed to parse message, disconnecting");
            }
            // the first message must always be "authenticate"
            else if (!is_authenticated()) {
                if (!handle_authentication(shared_from_this(), m)) {
                    this->disconnect("session failed to authenticate");
                }
            }
            else if (!handle_incoming_message(shared_from_this(), m)) {
                this->disconnect("[E] [SESSION] failed to process request: " + m->name());
            }
        }
        if (!is_disconnected_) {
            this->async_read_next_message();
        }
    };

    boost::asio::async_read_until(
        socket_,
        m->read_buffer(),
        message_matcher(),
        callback);
}
