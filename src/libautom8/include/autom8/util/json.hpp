#ifndef __C_AUTOM8_JSON_HPP__
#define __C_AUTOM8_JSON_HPP__

#include <json/writer.h>
#include <json/reader.h>
#include <json/value.h>

#include <boost/shared_ptr.hpp>

namespace autom8 {
    namespace json = Json;

    typedef json::Value json_value;
    typedef json::Reader json_reader;
    typedef json::FastWriter json_writer;
    typedef boost::shared_ptr<Json::Value> json_value_ref;

    extern std::string json_value_to_string(const json_value& value);
    extern json_value_ref json_value_from_string(const std::string& input);
    extern json_value string_vector_to_json_array(const std::vector<std::string>& input);
}

#endif
