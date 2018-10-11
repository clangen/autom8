#include <autom8/constants.h>
#include <autom8/net/session.hpp>
#include <autom8/util/debug.hpp>
#include <autom8/message/request_handler_factory.hpp>
#include <autom8/message/common_messages.hpp>
#include <autom8/net/server.hpp>
#include <autom8/util/utility.hpp>
#include <autom8/message/message_matcher.hpp>

#include <boost/format.hpp>
#include <atomic>

using namespace autom8;
using namespace nlohmann;

static std::atomic<int> instance_count_ { 0 };
static const std::string TAG = "session";

inline void print_instance_count() {
    //debug::log((boost::format("[D] session instance count: %1%") % instance_count_).str());
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
, is_disconnected_(true)
, write_queue_(new message_queue())
, wait_for_io_threads_(2)
{
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
    if (read_thread_ || write_thread_) {
        disconnect("[E] [SESSION] session already started, but start() called. disconnecting now.");
        return;
    }

    is_disconnected_ = false;
    ip_address_ = (socket_.lowest_layer().remote_endpoint().address().to_string());

    read_thread_.reset(new std::thread(
        std::bind(&session::read_thread_proc, this)));

    write_thread_.reset(new std::thread(
        std::bind(&session::write_thread_proc, this)));
}

std::string session::ip_address() const {
    return ip_address_;
}

bool session::is_authenticated() const {
    return is_authenticated_;
}

void session::enqueue_write(message_formatter_ptr formatter) {
    write_queue_->push(formatter);
}

bool session::handle_authentication(session_ptr session, message_ptr message) {
    if ((message->type() == message::message_type_request)) {
        if (message->name() == "authenticate") {
            std::string stored_password;
            utility::prefs().get("password", stored_password);

            auto document = message->body();
            std::string received_password = document["password"];

            if (session->is_authenticated_ = (stored_password == received_password)) {
                server::send(session, messages::responses::authenticated());
                return true;
            }
            else {
                debug::log(debug::error, TAG, "authenticate password mismatch");
            }
        }
        else {
            debug::log(debug::error, TAG, "expected authenticate, but got: " + message->name());
        }
    }
    else {
        debug::log(debug::error, TAG, "expected request, but got response");
    }

    // send failed response immediately, then disconnect
    {
        std::unique_lock<decltype(session->write_lock_)> lock(session->write_lock_);

        message_formatter_ptr f =
            message_formatter::create(
                messages::responses::authenticate_failed());

        write(session->socket_, boost::asio::buffer(f->to_string()));
    }

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

                debug::log(debug::info, TAG, "recv: autom8://request/" + m->name());
                request_handler_factory_ptr factory = request_handler_factory::instance();
                if ( ! factory->handle_request(session, m)) {
                    return false;
                }
            }
            return true;

        case message::message_type_response:
            {
                if (m->name() != "pong") { /* these pollute the the log */
                    debug::log(debug::info, TAG, "recv: autom8://response/" + m->name());
                }
            }
            return true;
    }

    return false;
}

void session::on_disconnected() {
    disconnect_signal_(shared_from_this());
}

void session::disconnect(const std::string& reason) {
    if ( ! is_disconnected_) {
        if (reason.size()) {
            debug::log(debug::info, TAG, reason);
        }

        is_disconnected_ = true;

        try {
            socket_.lowest_layer().close();
        }
        catch(...) {
            debug::log(debug::warning, TAG, "failed to close() socket");
        }

        write_queue_->stop();
    }

    on_disconnected();
}

void session::read_thread_proc() {
    session_ptr session = shared_from_this();

    wait_for_io_threads_.wait();

    try {
        socket_.handshake(boost::asio::ssl::stream_base::server);

        debug::log(debug::info, TAG, "session established: " + ip_address_);

        message_ptr m;

        while ( ! is_disconnected_) {
            m.reset(new message());
            boost::system::error_code error;

            size_t bytes_read = read_until(
                socket_, m->read_buffer(), message_matcher(), error);

            if (error) {
                disconnect("[E] [SESSION] socket read() failed");
            }
            else if (bytes_read > 0) {
                if ( ! m->parse_message(bytes_read)) {
                    disconnect("[E] [SESSION] failed to parse message, disconnecting");
                }
                // the first message must always be "authenticate"
                else if ( ! is_authenticated()) {
                    if ( ! handle_authentication(session, m)) {
                        disconnect("[E] [SESSION] session failed to authenticate");
                    }
                }
                else if ( ! handle_incoming_message(session, m)) {
                    disconnect("[E] [SESSION] failed to process request: " + m->name());
                }
            }
        }
    }
    catch (...) {
        disconnect("[E] [SESSION] exception caught, session disconnecting");
    }
}

void session::write_thread_proc() {
    session_ptr session = shared_from_this();

    wait_for_io_threads_.wait();

    try {
        message_queue_ptr q = write_queue_;
        message_formatter_ptr f;

        while (( ! is_disconnected_) && (f = q->pop_top())) {
            /*
             * Looks like someone is trying to make us do something fishy, we shouldn't
             * ever need to write() at this point until we're authenticated.
             */
            if ( ! is_authenticated_) {
                disconnect("[E] [SESSION] trying to write() when not authenticated");
                return;
            }

            std::unique_lock<decltype(session->write_lock_)> lock(session->write_lock_);
            write(socket_, boost::asio::buffer(f->to_string()));
        }
    }
    catch (message_queue::stopped_exception&) {
        disconnect("[I] [SESSION] write_queue_ stopped, disconnecting");
    }
    catch (...) {
        disconnect("[E] [SESSION] write() failed, disconnecting");
    }
}