#include <autom8/device/device_base.hpp>
#include <boost/format.hpp>

#include <json/value.h>
#include <json/writer.h>

using namespace autom8;

json_value_ref device_base::to_json() {
    json_value_ref result(new Json::Value(Json::objectValue));

    (*result)["address"] = Json::Value(this->address());
    (*result)["type"] = Json::Value(this->type());
    (*result)["label"] = Json::Value(this->label());
    (*result)["status"] = Json::Value(this->status());

    std::vector<std::string> groups;
    this->groups(groups);
    (*result)["groups"] = autom8::string_vector_to_json_array(groups);

    (*result)["attributes"] = Json::Value(Json::objectValue);
    this->get_extended_json_attributes((*result)["attributes"]);

    return result;
}

void device_base::get_extended_json_attributes(json_value& target) {
}

void device_base::on_status_changed() {
    status_changed(shared_from_this());
}