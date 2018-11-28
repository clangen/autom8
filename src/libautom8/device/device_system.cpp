#include <f8n/debug/debug.h>
#include <f8n/str/util.h>

#include <autom8/device/device_system.hpp>
#include <autom8/device/null_device_system.hpp>
#include <autom8/device/x10/mochad/mochad_device_system.hpp>
#ifdef WIN32
#include <autom8/device/x10/cm15a/cm15a_device_system.hpp>
#endif

#include <mutex>

using namespace autom8;
using namespace f8n;

static const std::string TYPE_NULL = "null";
static const std::string TYPE_MOCHAD = "mochad";
#ifdef WIN32
static const std::string TYPE_CM15A = "cm15a";
static const std::string DEFAULT_TYPE = TYPE_CM15A;
#else
static const std::string DEFAULT_TYPE = TYPE_MOCHAD;
#endif

static std::mutex instance_mutex_;
static device_system_ptr null_device_system_;
static device_system_ptr instance_;
static std::string type = TYPE_NULL;

device_system_ptr device_system::instance() {
    std::unique_lock<decltype(instance_mutex_)> lock(instance_mutex_);
    if (!instance_) {
        if (!null_device_system_) {
            null_device_system_ = std::make_shared<null_device_system>();
        }
        return null_device_system_;
    }
    return instance_;
}

device_system_ptr device_system::set_instance(const std::string& type) {
    std::unique_lock<decltype(instance_mutex_)> lock(instance_mutex_);
    if (::type != type) {
        if (type == TYPE_NULL) {
            instance_ = std::make_shared<null_device_system>();
        }
        else if (type == TYPE_MOCHAD) {
            instance_ = std::make_shared<mochad_device_system>();
        }
#ifdef WIN32
        else if (type == TYPE_CM15A) {
            instance_ = std::make_shared<cm15a_device_system>();
        }
#endif
        else {
            debug::e("device_system", str::format("invalid device system '%s' specified", type.c_str()));
            return instance_;
        }
        ::type = type;
    }
    return instance_;
}

std::vector<std::string> device_system::types() {
    return {
        TYPE_NULL,
        TYPE_MOCHAD,
#ifdef WIN32
        TYPE_CM15A
#endif
    };
}

std::string device_system::default_type() {
    return DEFAULT_TYPE;
}

std::string device_system::current_type() {
    std::unique_lock<decltype(instance_mutex_)> lock(instance_mutex_);
    return ::type;
}

void device_system::clear_instance() {
    std::unique_lock<decltype(instance_mutex_)> lock(instance_mutex_);
    instance_ = device_system_ptr();
}