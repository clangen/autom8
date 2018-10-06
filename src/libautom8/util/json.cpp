#include <autom8/util/json.hpp>
#include <boost/lexical_cast.hpp>

namespace autom8 {
    std::string json_value_to_string(const json_value& value) {
        switch (value.type()) {
        case json::stringValue:
        case json::nullValue:
        case json::booleanValue:
            return value.asString();

        case json::intValue:
            return boost::lexical_cast<std::string>(value.asInt());

        case json::uintValue:
            return boost::lexical_cast<std::string>(value.asUInt());

        case json::realValue:
            return boost::lexical_cast<std::string>(value.asDouble());

        default:
            json_writer writer;
            return writer.write(value);
        }
    }

    json_value_ref json_value_from_string(const std::string& str) {
        json_reader reader;
        json_value_ref result = json_value_ref(new json_value());

        if (reader.parse(str.c_str(), *result)) {
            return result;
        }

        return json_value_ref();
    }

    json_value string_vector_to_json_array(const std::vector<std::string>& input) {
        json_value result = json_value(Json::arrayValue);

        for (size_t i = 0; i < input.size(); i++) {
            result.append(input[i]);
        }

        return result;
    }
}