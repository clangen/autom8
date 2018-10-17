#include <autom8/constants.h>
#include <autom8/message/request_handler_registrar.hpp>
#include <autom8/net/server.hpp>
#include <autom8/net/session.hpp>
#include <autom8/util/ssl_certificate.hpp>
#include <autom8/message/common_messages.hpp>
#include <f8n/debug/debug.h>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <base64/base64.h>
#include <ostream>
#include <functional>
#include <condition_variable>

using namespace autom8;
using debug = f8n::debug;

static server::server_ptr instance_;
static std::mutex instance_mutex_;
sigslot::signal0<> server::started;
sigslot::signal0<> server::stopped;

static const std::string TAG = "server";

typedef boost::format format;

server::server(unsigned short port)
: endpoint_(tcp::v4(), port)
, io_service_()
, acceptor_(io_service_, endpoint_)
, ssl_context_(boost::asio::ssl::context::sslv23)
, stopped_(false) {
    if (!ssl_certificate::exists()) {
        if (!ssl_certificate::generate()) {
            debug::error(TAG, "unable to generate SSL certificate! aborting!");
            throw std::exception();
        }
    }

    try {
        ssl_context_.use_certificate_chain_file(ssl_certificate::filename());
        ssl_context_.use_private_key_file(ssl_certificate::filename(), boost::asio::ssl::context::pem);
        ssl_context_.set_options(boost::asio::ssl::context::verify_none);
    }
    catch (...) {
        debug::error(TAG, "ssl certificate generated, apparently, but unable to load?");
        ssl_certificate::remove();
        throw std::exception();
    }
}

server::~server() {
}

void server::start_instance() {
    debug::info(TAG, "started");

    io_service_thread_.reset(new std::thread(
        std::bind(&server::io_service_thread_proc, this)));
}

void server::stop_instance() {
    if (ping_timer_) {
        ping_timer_->cancel();
    }

    // wait for the acceptor and io service to stop, no more connections
    // will be processed.
    stopped_ = true;
    acceptor_.close();

    /*
    ** alright, this is a bit strange. to avoid deadlock we first make a copy
    ** of the active session map to a local stack variable, then, within the
    ** lock, clear all our references to the active session_ptrs. once the lock
    ** has been released, we disconnect the sessions from the local copy.
    */
    {
        session_list active_sessions;

        {
            std::unique_lock<decltype(state_mutex_)> lock(state_mutex_);
            active_sessions = session_list_;
            session_list_.clear();
        }

        session_list::iterator it = active_sessions.begin();
        while (it != active_sessions.end()) {
            (*it)->disconnect("session disconnecting, server shutting down");
             ++it;
        }
    }

    io_service_.stop();
    io_service_thread_->join();

    debug::info(TAG, "stopped");
}

void server::io_service_thread_proc() {
    start_accept();     /* async tcp socket accept () */
    schedule_ping();    /* heartbeat */
    io_service_.run();  /* stopped by stop_instance */
}

void server::start_accept() {
    session_ptr sess(new session(io_service_, ssl_context_));

    acceptor_.async_accept(
        sess->socket().lowest_layer(),
        boost::bind(
            &server::handle_accept,
            this,
            boost::asio::placeholders::error,
            sess
        )
    );
}

void server::handle_accept(const boost::system::error_code& error, session_ptr sess) {
    if (error) {
        debug::info(TAG, "socket accept failed, connection error or server shutting down");
        return;
    }

    boostrap_new_session(sess);
    start_accept();
}

bool server::start(unsigned short port) {
    std::unique_lock<decltype(instance_mutex_)> lock(instance_mutex_);

    if (!instance_) {
        request_handler_registrar::register_all();

        instance_ = server_ptr(new server(port));
        instance_->start_instance();
        server::started();

        return true;
    }

    return false;
}

bool server::stop() {
    server_ptr s;

    /*
    ** The stop_instance() operation blocks until all sessions have disconnected,
    ** and the session's disconnect() waits until all read and write threads have
    ** finished. the read and write threads may be in the middle of something, and
    ** try to call server::send() after this lock is aquired locally in this method,
    ** and sever::send() will acquire the same lock, resulting in a deadlock. we
    ** work around this by caching the instance variable on the stack and resetting
    ** instance_ before calling stop_instance().
    */
    {
        std::unique_lock<decltype(instance_mutex_)> lock(instance_mutex_);
        s = instance_;
        instance_ = server_ptr();
    }

    if (s) {
        s->stop_instance();
        server::stopped();
    }

    return true;
}

bool server::is_running() {
    std::unique_lock<decltype(instance_mutex_)> lock(instance_mutex_);
    return (instance_ ? true : false);
}

void server::send(session_ptr session, response_ptr response) {
    server_ptr server;

    {
        std::unique_lock<decltype(instance_mutex_)> lock(instance_mutex_);
        server = instance_;
    }

    if (server) {
        server->dispatch_response(session, response);
    }
}

void server::send(session_ptr session, request_ptr response) {
    server_ptr server;

    {
        std::unique_lock<decltype(instance_mutex_)> lock(instance_mutex_);
        server = instance_;
    }

    if (server) {
        server->dispatch_request(session, response);
    }
}

// returns false if the broadcast response fails; note this method has a side-
// effect of automatically disconnected any dead clients
void server::send(response_ptr response) {
    server::send(session_ptr(), response);
}

// dispatches the specified request to all connected clients; note this method has
// a side-effect of automatically disconnecting any clients that are connected
void server::send(request_ptr request) {
    server_ptr server;
    {
        std::unique_lock<decltype(instance_mutex_)> lock(instance_mutex_);
        server = instance_;
    }

    if (server) {
        std::unique_lock<decltype(server->state_mutex_)> lock(server->state_mutex_);

        session_list::iterator it = server->session_list_.begin();
        for ( ; it != server->session_list_.end(); it++) {
            server::send(*it, request);
        }
    }
}

void server::handle_scheduled_ping(const error_code& error) {
    ping_timer_ = timer_ptr();
    if ((!error) && (!stopped_)) {
        server::send(messages::requests::ping());
        schedule_ping();
    }
}

void server::schedule_ping() {
    if (ping_timer_) {
        ping_timer_->cancel();
    }

    ping_timer_ = timer_ptr(new boost::asio::deadline_timer(io_service_));
    ping_timer_->expires_from_now(boost::posix_time::seconds(240));

    ping_timer_->async_wait(
        bind(
            &server::handle_scheduled_ping,
            this,
            boost::asio::placeholders::error));
}

void server::on_session_disconnected(session_ptr session) {
    std::unique_lock<decltype(state_mutex_)> lock(state_mutex_);

    session_list::iterator it = session_list_.find(session);
    if (it != session_list_.end()) {
        session_list_.erase(it);

        // there was a bug here where we were trying to access socket.lowest_layer.remote_endpoint
        // after the socket had been shut down, which sometimes could raise an exception. now the
        // ip address is cached in the session instance itself
        debug::info(
            TAG,
            "session disconnected: " + session->ip_address() + ", " +
            (format("count=%1%") % session_list_.size()).str());
    }
}

void server::dispatch_request(session_ptr session, request_ptr request) {
    if (!session) {
        debug::error(TAG, "can't dispatch request: no session specified");
        return;
    }

    session->enqueue_write(message_formatter::create(request));

    // suppress the pings, they just pollute the log file.
    std::string uri = request->uri();
    if (uri != "autom8://request/ping") {
        debug::info(TAG, "send: " + uri);
    }
}

void server::dispatch_response(session_ptr session, response_ptr response) {
    // if no session was specified, and the response type is target only,
    // we can't send the message. someone messed up. TODO: throw?
    if ((!session) && (response->target() == response::requester_only)) {
        debug::error(TAG, "can't dispatch message to requester, no session specified");
        return;
    }

    message_formatter_ptr formatter =
        message_formatter::create(response);

    switch (response->target()) {
    case response::requester_only:
        session->enqueue_write(formatter);
        break;

    case response::all_sessions:
    case response::all_other_sessions: {
            std::unique_lock<decltype(state_mutex_)> lock(state_mutex_);
            session_list::iterator it = session_list_.begin();
            for ( ; it != session_list_.end(); it++) {
                if ((response->target() == response::all_other_sessions)
                && ((*it) == session)) {
                    continue;
                }

                (*it)->enqueue_write(formatter);
            }
        }
        break;
    }

    // suppress the pings, they just pollute the log file.
    std::string uri = response->uri();
    if (uri != "autom8://response/pong") {
        debug::info(TAG, "send: " + uri);
    }
}

void server::boostrap_new_session(session_ptr session) {
    {
        std::unique_lock<decltype(state_mutex_)> lock(state_mutex_);
        session_list_.insert(session);
        session->disconnect_signal().connect(this, &server::on_session_disconnected);
        session->start();
        debug::info(TAG, (format("session starting, count=%1%") % session_list_.size()).str());
    }

    /* implicitly wakes up idle sessions, and will cause the OS to detect
    and close dead sockets, cleaning up stale instances. */
    server::send(messages::requests::ping());
}
