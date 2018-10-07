#pragma once

#include <boost/asio.hpp>
#include <json.hpp>

namespace autom8 {
    class message {
    private:
        typedef boost::asio::streambuf::const_buffers_type buffer_type;
        typedef boost::asio::buffers_iterator<buffer_type> buffer_iterator;
        typedef boost::asio::streambuf streambuf;

    public:
        enum message_type {
            message_type_invalid = -1,
            message_type_request = 0,
            message_type_response = 1,
        };

        message();

        const std::string& name() const;
        const nlohmann::json& body() const;
        message_type type() const;

        streambuf& read_buffer();
        bool parse_message(size_t bytes_read);

    private:
        void read_string_from_buffer(std::string& target, size_t bytes_read);

        streambuf read_buffer_;
        std::string name_;
        nlohmann::json body_;
        message_type type_;
    };

    typedef std::shared_ptr<message> message_ptr;
}
