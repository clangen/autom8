#include <autom8/device/null_device_system.hpp>
#include <autom8/device/device_base.hpp>
#include <autom8/device/simple_device.hpp>
#include <autom8/net/server.hpp>
#include <autom8/message/common_messages.hpp>

using namespace autom8;

class null_device: public simple_device, public lamp, public security_sensor {
public:
    null_device(
        database_id id,
        const std::string& address,
        const std::string& label,
        const std::vector<std::string>& groups = std::vector<std::string>(),
        device_type type = device_type_unknown)
        : simple_device(id, address, label, groups)
        , type_(type)
        , brightness_(100)
        , armed_(false)
        , tripped_(false)
    {
        status_ = device_status_off;

        if (type == device_type_security_sensor) {
            armed_ = true;
        }
    }

    virtual void on_status_changed() {
        simple_device::on_status_changed();
        server::send(messages::responses::device_status_updated(shared_from_this()));

        if (type_ == device_type_security_sensor) {
            server::send(messages::responses::sensor_status_changed(shared_from_this()));
        }
    }

    /* device base */
    void get_extended_json_attributes(json_value& target) {
        boost::recursive_mutex::scoped_lock lock(state_mutex());

        if (type_ == device_type_lamp) {
            target["brightness"] = brightness_;
        }
        else if (type_ == device_type_security_sensor) {
            target["armed"] = armed_;
            target["tripped"] = tripped_;
        }
    }

    /* simple device */
    virtual device_type type() {
        boost::recursive_mutex::scoped_lock lock(state_mutex());
        return type_;
    }

    virtual void turn_on() {
        bool changed = false;

        {
            boost::recursive_mutex::scoped_lock lock(state_mutex());

            if (status_ != device_status_on) {
                status_ = device_status_on;
                changed = true;
            }
        }

        if (changed) {
            on_status_changed();
        }
    }

    virtual void turn_off() {
        bool changed = false;

        {
            boost::recursive_mutex::scoped_lock lock(state_mutex());

            if (status_ != device_status_off) {
                status_ = device_status_off;
                changed = true;
            }
        }

        if (changed) {
            on_status_changed();
        }
    }

    /* lamp */
    virtual int brightness() {
        boost::recursive_mutex::scoped_lock lock(state_mutex());
        return brightness_;
    }

    virtual void set_brightness(int brightness) {
        bool changed = false;

        {
            boost::recursive_mutex::scoped_lock lock(state_mutex());

            if (brightness != brightness_) {
                brightness_ = brightness;
                changed = true;
            }
        }

        if (changed) {
            on_status_changed();
        }
    };

    /* security_sensor */
    virtual void arm() {
        bool changed = false;

        {
            boost::recursive_mutex::scoped_lock lock(state_mutex());

            if (!armed_ || status_ != device_status_on) {
                armed_ = true;
                status_ = device_status_on;
                changed = true;
            }
        }

        if (changed) {
            on_status_changed();
        }
    }

    virtual void disarm() {
        bool changed = false;

        {
            boost::recursive_mutex::scoped_lock lock(state_mutex());

            if (armed_ || status_ == device_status_on) {
                armed_ = false;
                status_ = device_status_off;
                changed = true;
            }
        }

        if (changed) {
            on_status_changed();
        }
    }

    virtual void reset() {
        bool changed = false;

        {
            boost::recursive_mutex::scoped_lock lock(state_mutex());
            if (tripped_) {
                tripped_ = false;
                changed = true;
            }
        }

        if (changed) {
            on_status_changed();
        }
    }

    virtual bool is_armed() {
        boost::recursive_mutex::scoped_lock lock(state_mutex());
        return armed_;
    }

    virtual bool is_tripped() {
        boost::recursive_mutex::scoped_lock lock(state_mutex());
        return tripped_;
    }

private:
    device_type type_;
    int brightness_;
    bool armed_, tripped_;
};

typedef std::map<std::string, device_ptr> addr_map;
addr_map addr_to_dev_;
boost::mutex device_map_mutex_;

null_device_system::null_device_system()
: model_(device_factory_ptr(new null_device_factory()))
{
    model_.device_updated.connect(
        this, &null_device_system::on_device_updated
    );

    model_.device_removed.connect(
        this, &null_device_system::on_device_removed
    );
}

device_ptr null_device_system::null_device_factory::create(
    database_id id,
    device_type type,
    const std::string& address,
    const std::string& label,
    const std::vector<std::string>& groups)
{
    boost::mutex::scoped_lock lock(device_map_mutex_);

    if (addr_to_dev_.find(address) != addr_to_dev_.end()) {
        device_ptr existing = addr_to_dev_.find(address)->second;
        if (existing && existing->type() == type) {
            simple_device* simple = (simple_device*) existing.get();
            simple->update(address, label, groups);
            return existing;
        }
    }

    device_ptr device = device_ptr(
        new null_device(id, address, label, groups, type)
    );

    addr_to_dev_[address] = device;

    return device;
}

std::string null_device_system::null_device_factory::name() const {
    return "null";
}

void null_device_system::on_device_removed(database_id id) {
    boost::mutex::scoped_lock lock(device_map_mutex_);

    addr_map::iterator it;
    bool found = false;
    for (it = addr_to_dev_.begin(); it != addr_to_dev_.end(); it++) {
        if (it->second->id() == id) {
            found = true;
            break;
        }
    }

    if (found) {
        addr_to_dev_.erase(it);
    }
}

void null_device_system::on_device_updated(database_id id) {
}