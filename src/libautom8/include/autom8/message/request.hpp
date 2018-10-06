#ifndef __C_AUTOM8_REQUEST_HPP__
#define __C_AUTOM8_REQUEST_HPP__

#include <autom8/util/json.hpp>

#include <string>
#include <boost/shared_ptr.hpp>

namespace autom8 {
    class request;
    typedef boost::shared_ptr<request> request_ptr;

    class request {
    public:
        static request_ptr create(const std::string& uri, json_value_ref body);
        virtual std::string uri() = 0;
        virtual json_value_ref body() = 0;
    };


}

#endif