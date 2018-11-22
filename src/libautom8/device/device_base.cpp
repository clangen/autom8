#include <autom8/device/device_base.hpp>
#include <boost/format.hpp>
#include <json.hpp>

using namespace autom8;
using namespace nlohmann;

json device_base::to_json() {
    json result;

    result["address"] = this->address();
    result["type"] = this->type();
    result["label"] = this->label();
    result["status"] = this->status();
    result["groups"] = this->groups();

    json attributes = json::object();
    this->get_extended_json_attributes(attributes);
    result["attributes"] = attributes;

    return result;
}

void device_base::get_extended_json_attributes(json& target) {
}

void device_base::on_status_changed() {
    status_changed(shared_from_this());
}