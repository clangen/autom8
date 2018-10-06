#ifndef __C_AUTOM8_MESSAGE_FORMATTER_HPP__
#define __C_AUTOM8_MESSAGE_FORMATTER_HPP__

#include <boost/shared_ptr.hpp>
#include <base64/base64.h>
#include <autom8/constants.h>
#include <autom8/message/request.hpp>
#include <autom8/message/response.hpp>

namespace autom8 {
    class message_formatter;
    typedef boost::shared_ptr<message_formatter> message_formatter_ptr;

    class message_formatter {
    public:
        static message_formatter_ptr create(response_ptr response) {
            return message_formatter_ptr(new message_formatter(response));
        }

        static message_formatter_ptr create(request_ptr request) {
            return message_formatter_ptr(new message_formatter(request));
        }

        const std::string& to_string() {
            return formatted_;
        }

    private:
        message_formatter(response_ptr response) {
            build_message_string(response->uri(), response->body());
        }

        message_formatter(request_ptr request) {
            build_message_string(request->uri(), request->body());
        }

    private:
        void build_message_string(const std::string& uri, json_value_ref body) {
            json_writer writer;
            std::string body_string = writer.write(*body);
            std::string message_string = uri + MESSAGE_URI_DELIMITER + body_string;
            formatted_ = base64_encode(message_string) + ((char) END_OF_MESSAGE);
        }

    private:
        std::string formatted_;
    };

} // namespace autom8

#endif