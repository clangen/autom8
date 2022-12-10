#include <autom8/message/message.hpp>
#include <autom8/constants.h>

#include <f8n/debug/debug.h>
#include <f8n/str/util.h>

#include <base64/base64.h>

#include <regex>

using namespace nlohmann;
using namespace autom8;

using debug = f8n::debug;
using message_type = message::message_type;

static const std::string TAG = "message";

message::message()
: type_(message_type_invalid) {
}

asio::streambuf& message::read_buffer() {
    return read_buffer_;
}

const json& message::body() const {
    return body_;
}

message_type message::type() const {
    return type_;
}

const std::string& message::name() const {
    return name_;
}

bool message::parse_message(size_t bytes_read) {
    std::string plain_text;

    try {
        // read message
        std::string base64_text;
        read_string_from_buffer(base64_text, bytes_read);

        // base 64 decode
        plain_text = base64_decode(base64_text);

        // parse the message. it should consist of two parts: a URI and
        // a body, separated by a MESSAGE_URI_DELIMITER
        std::vector<std::string> split_result = f8n::str::split(plain_text, MESSAGE_URI_DELIMITER);
        if (split_result.size() != 2) {
            return false;
        }
        //
        std::string uri = split_result[0];
        std::string body = split_result[1];

        // make sure the body is a valid JSON document
        body_ = json::parse(body);

        // parse and validate the URI
        std::cmatch uri_matches;
        std::regex uri_regex(MESSAGE_URI_REGEX_MATCH);
        if (std::regex_match(uri.c_str(), uri_matches, uri_regex)) {
            if (uri_matches.size() == 4) {
                // [0] = full string
                // [1] = autom8://
                std::string type_str = uri_matches[2];
                std::string name_str = uri_matches[3];

                if (type_str == MESSAGE_URI_TYPE_REQUEST) {
                    this->type_ = message_type_request;
                }
                else if (type_str == MESSAGE_URI_TYPE_RESPONSE) {
                    this->type_ = message_type_response;
                }
                else {
                    return false;
                }

                name_ = name_str;
                return true;
            }
        }
    }
    catch (...) {
        /* swallow, we'll report an error below... */
    }

	debug::error(TAG, "could not parse message! " + plain_text);
    return false;
}

void message::read_string_from_buffer(std::string& target, size_t bytes_read) {
    buffer_type buffers = read_buffer_.data();
    buffer_iterator buff_it = asio::buffers_begin(buffers);

    target.assign(buff_it, (buff_it + bytes_read));

    read_buffer_.consume(bytes_read);
}
