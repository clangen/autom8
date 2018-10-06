#ifndef __C_AUTOM8_RESPONSE_HPP__
#define __C_AUTOM8_RESPONSE_HPP__

#include <autom8/message/message.hpp>
#include <autom8/util/json.hpp>

namespace autom8 {
    class response;
    typedef boost::shared_ptr<response> response_ptr;

    class response {
    public:
        enum response_target {
            requester_only = 0,
            all_sessions,
            all_other_sessions
        };

        static response_ptr create(
            const std::string& uri,
            json_value_ref body,
            response_target target = requester_only);

        virtual std::string uri() = 0;
        virtual json_value_ref body() = 0;
        virtual response_target target() = 0;
    };
}

#endif