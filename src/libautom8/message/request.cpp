#include <autom8/message/request.hpp>

using namespace autom8;
using namespace nlohmann;

class generic_request: public request {
public:
    generic_request(
        const std::string& uri,
        const json& body)
    {
        uri_ = uri;
        body_ = std::make_shared<json>(body);
    }

    virtual std::string uri() { return uri_; }
    virtual const std::shared_ptr<json> body() { return body_; }

private:
    std::string uri_;
    std::shared_ptr<json> body_;
};

request_ptr request::create(const std::string& uri, const nlohmann::json& body) {
    return request_ptr(new generic_request(uri, body));
}