#pragma once

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/high_resolution_timer.hpp>
#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <sigslot/sigslot.h>

namespace autom8 {
    class mochad_controller {
    public:
        sigslot::signal1<std::string> message_received;

        mochad_controller();
        ~mochad_controller();

        bool send(std::string msg);
        bool requery(const std::string& device_address);
        void restart();

        static std::string default_hostname() { return "localhost"; }
        static int default_port() { return 1099; }

    private:
        bool init();
        void deinit();

        void io_thread_proc();

        void start_connecting();

        void handle_resolve(
            const std::error_code&,
            asio::ip::tcp::resolver::iterator
        );

        void handle_connect(
            const std::error_code&,
            asio::ip::tcp::resolver::iterator
        );

        void handle_write(const std::error_code&);
        void handle_read(const std::error_code&, size_t);
        void read_next_message();
        void start_next_write();
        void disconnect();
        void schedule_reconnect();

        void schedule_ping();
        void send_ping();

    private:
        asio::io_service io_service_;
        asio::ip::tcp::socket* socket_;
        asio::ip::tcp::resolver resolver_;
        asio::streambuf read_buffer_;
        asio::high_resolution_timer reconnect_timer_, ping_timer_;
        std::shared_ptr<std::thread> io_thread_;
        std::queue<std::string> write_queue_;
        std::mutex write_queue_lock_, connection_lock_;
        volatile bool initialized_, connected_, reconnecting_, writing_;
    };
}
