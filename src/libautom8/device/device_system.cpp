#include <autom8/device/device_system.hpp>
#include <autom8/device/null_device_system.hpp>
#include <mutex>

using namespace autom8;

static std::mutex instance_mutex_;
static device_system_ptr null_device_system_ = device_system_ptr(new null_device_system());
static device_system_ptr instance_;

device_system_ptr device_system::instance() {
    std::unique_lock<decltype(instance_mutex_)> lock(instance_mutex_);

    if ( ! instance_) {
        return null_device_system_;
    }

    return instance_;
}

device_system_ptr device_system::set_instance(device_system_ptr new_instance) {
    {
        std::unique_lock<decltype(instance_mutex_)> lock(instance_mutex_);
        instance_ = new_instance;
    }

    return instance();
}

void device_system::clear_instance() {
    device_system::set_instance(device_system_ptr());
}