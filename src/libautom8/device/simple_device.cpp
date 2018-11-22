#include <vector>
#include <string>
#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <autom8/message/common_messages.hpp>
#include <autom8/device/simple_device.hpp>

using namespace autom8;

simple_device::simple_device(
    database_id id,
    const std::string& address,
    const std::string& label,
    const std::vector<std::string>& groups)
: device_base()
, id_(id)
, address_(address)
, label_(label)
, status_(device_status_unknown)
, groups_(groups) {
    /* nothing */
}

simple_device::~simple_device() {
}

std::vector<std::string> simple_device::groups() {
    std::vector<std::string> target;
    auto lock = state_lock();
    std::vector<std::string>::iterator it = groups_.begin();
    for ( ; it != groups_.end(); it++) {
        target.push_back(*it);
    }
    return target;
}

device_status simple_device::status() {
    auto lock = state_lock();
    return status_;
}

std::string simple_device::address() {
    auto lock = state_lock();
    return address_;
}

std::string simple_device::label() {
    auto lock = state_lock();
    return label_;
}

database_id simple_device::id() {
    auto lock = state_lock();
    return id_;
}

void simple_device::on_updated() {
}

void simple_device::update(
    const std::string& new_address,
    const std::string& new_label)
{
    {
        auto lock = state_lock();
        address_ = new_address;
        label_ = new_label;
    }

    this->on_updated();
}

void simple_device::update(
    const std::string& new_address,
    const std::string& new_label,
    const std::vector<std::string>& groups)
{
    {
        auto lock = state_lock();
        address_ = new_address;
        label_ = new_label;
    }

    set_groups(groups);
    this->on_updated();
}

void simple_device::set_groups(const std::vector<std::string>& groups) {
    {
        auto lock = state_lock();
        groups_.assign(groups.begin(), groups.end());
    }

    this->on_updated();
}

std::recursive_mutex& simple_device::state_mutex() {
    return state_mutex_;
}

std::unique_lock<std::recursive_mutex> simple_device::state_lock() {
    return std::unique_lock<std::recursive_mutex>(state_mutex_);
}