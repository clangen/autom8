#include <autom8/message/request.hpp>

using namespace autom8;

class generic_request: public request {
public:
    generic_request(
        const std::string& uri,
        const json_value_ref body)
    {
        uri_ = uri;
        body_ = body;
    }

    virtual std::string uri() { return uri_; }
    virtual json_value_ref body() { return body_; }

private:
    std::string uri_;
    json_value_ref body_;
};

request_ptr request::create(const std::string& uri, json_value_ref body) {
    return request_ptr(new generic_request(uri, body));
}