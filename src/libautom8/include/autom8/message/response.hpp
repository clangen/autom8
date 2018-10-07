#pragma once

#include <autom8/message/message.hpp>
#include <json.hpp>
#include <memory>

namespace autom8 {
    class response;
    typedef std::shared_ptr<response> response_ptr;

    class response {
    public:
        enum response_target {
            requester_only = 0,
            all_sessions,
            all_other_sessions
        };

        static response_ptr create(
            const std::string& uri,
            const nlohmann::json& body,
            response_target target = requester_only);

        virtual std::string uri() = 0;
        virtual std::shared_ptr<nlohmann::json> body() = 0;
        virtual response_target target() = 0;
    };
}
