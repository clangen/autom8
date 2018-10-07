#include <autom8/message/response.hpp>

using namespace autom8;
using namespace nlohmann;

class generic_response: public response {
public:
    generic_response(
        const std::string& uri,
        const nlohmann::json& body,
        response_target target)
    {
        uri_ = uri;
        body_ = std::make_shared<json>(body);
        target_ = target;
    }

    static response_ptr create(
        const std::string& uri,
        const nlohmann::json& body,
        response_target target)
    {
        return response_ptr(new generic_response(uri, body, target));
    }

    static response_ptr create(
        const std::string& uri,
        const nlohmann::json& body)
    {
        return response_ptr(new generic_response(uri, body, response::requester_only));
    }

    virtual std::string uri() { return uri_; }
    virtual std::shared_ptr<nlohmann::json> body() { return body_; }
    virtual response_target target() { return target_; }

private:
    std::string uri_;
    std::shared_ptr<nlohmann::json> body_;
    response::response_target target_;
};

response_ptr response::create(const std::string& uri, const nlohmann::json& body, response_target target) {
    return response_ptr(new generic_response(uri, body, target));
}
