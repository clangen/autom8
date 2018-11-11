#pragma once

#include <json.hpp>
#include <string>
#include <memory>

namespace autom8 {
    class request;
    typedef std::shared_ptr<request> request_ptr;

    class request {
    public:
        static request_ptr create(const std::string& uri, const nlohmann::json& body = nlohmann::json());
        virtual std::string uri() = 0;
        virtual const std::shared_ptr<nlohmann::json> body() = 0;
    };


}
