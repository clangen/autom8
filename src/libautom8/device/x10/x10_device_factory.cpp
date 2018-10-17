#include <autom8/device/x10/x10_device_factory.hpp>
#include <autom8/device/x10/x10_device_system.hpp>
#include <autom8/device/x10/x10_appliance.hpp>
#include <autom8/device/x10/x10_lamp.hpp>
#include <autom8/device/x10/x10_security_sensor.hpp>

using namespace autom8;

x10_device_factory::x10_device_factory(x10_device_system* owner)
: owner_(owner) {
}

device_ptr x10_device_factory::create(
    database_id id,
    device_type type,
    const std::string& address,
    const std::string& label,
    const std::vector<std::string>& groups)
{
    {
        std::unique_lock<decltype(id_device_map_mutex_)> lock(id_device_map_mutex_);

        id_device_map::iterator it = id_device_map_.find(id);
        if (it != id_device_map_.end()) {
            device_ptr device = it->second;

            if (type == device->type()) {
                x10_device* x10 = (x10_device*) device.get();
                x10->update(address, label, groups);
                return it->second;
            }
        }
    }

    x10_device* raw_device = NULL;
    device_ptr result;

    switch (type) {
    case device_type_lamp:
        raw_device = new x10_lamp(owner_, id, address, label);
        break;
    case device_type_appliance:
        raw_device = new x10_appliance(owner_, id, address, label);
        break;
    case device_type_security_sensor:
        raw_device = new x10_security_sensor(owner_, id, address, label);
        break;

    default:
        break;
    }

    if (raw_device) {
        raw_device->set_groups(groups);
        result = device_ptr(raw_device);
    }

    if (result) {
        std::unique_lock<decltype(id_device_map_mutex_)> lock(id_device_map_mutex_);
        id_device_map_[id] = result;
    }

    return result;
}

std::string x10_device_factory::name() const {
    return owner_->controller_type();
}

void x10_device_factory::device_removed(database_id id) {
std::unique_lock<decltype(id_device_map_mutex_)> lock(id_device_map_mutex_);

    id_device_map::iterator it = id_device_map_.find(id);
    if (it != id_device_map_.end()) {
         id_device_map_.erase(it);
    }
}

void x10_device_factory::device_updated(database_id id) {

}
