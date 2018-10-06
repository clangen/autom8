#include <vector>
#include <string>
#include <iostream>

#include <autom8/device/x10/x10_appliance.hpp>

using namespace autom8;

x10_appliance::x10_appliance(
    x10_device_system* owner,
    database_id id,
    const std::string& x10_address,
    const std::string& label)
: x10_device(owner, id, x10_address, label) {
}

x10_appliance::~x10_appliance() {
}

device_type x10_appliance::type() {
    return device_type_appliance;
}
