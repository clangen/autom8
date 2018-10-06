#include <autom8/message/response.hpp>

using namespace autom8;

class generic_response: public response {
public:
    generic_response(
        const std::string& uri,
        const json_value_ref body,
        response_target target)
    {
        uri_ = uri;
        body_ = body;
        target_ = target;
    }

    static response_ptr create(
        const std::string& uri,
        const json_value_ref body,
        response_target target)
    {
        return response_ptr(new generic_response(uri, body, target));
    }

    static response_ptr create(
        const std::string& uri,
        const json_value_ref body)
    {
        return response_ptr(new generic_response(uri, body, response::requester_only));
    }

    virtual std::string uri() { return uri_; }
    virtual json_value_ref body() { return body_; }
    virtual response_target target() { return target_; }

private:
    std::string uri_;
    json_value_ref body_;
    response::response_target target_;
};

response_ptr response::create(const std::string& uri, json_value_ref body, response_target target) {
    return response_ptr(new generic_response(uri, body, target));
}
