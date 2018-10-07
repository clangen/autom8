#pragma once

#include <memory>
#include <base64/base64.h>
#include <autom8/constants.h>
#include <autom8/message/request.hpp>
#include <autom8/message/response.hpp>
#include <json.hpp>

namespace autom8 {
    class message_formatter;
    typedef std::shared_ptr<message_formatter> message_formatter_ptr;

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
            build_message_string(response->uri(), *response->body());
        }

        message_formatter(request_ptr request) {
            build_message_string(request->uri(), *request->body());
        }

    private:
        void build_message_string(const std::string& uri, const nlohmann::json& body) {
            std::string message_string = uri + MESSAGE_URI_DELIMITER + body.dump();
            formatted_ = base64_encode(message_string) + ((char) END_OF_MESSAGE);
        }

    private:
        std::string formatted_;
    };

} // namespace autom8
