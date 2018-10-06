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

void simple_device::groups(std::vector<std::string>& target) {
    boost::recursive_mutex::scoped_lock lock(state_mutex_);

    std::vector<std::string>::iterator it = groups_.begin();
    for ( ; it != groups_.end(); it++) {
        target.push_back(*it);
    }
}

device_status simple_device::status() {
    boost::recursive_mutex::scoped_lock lock(state_mutex_);
    return status_;
}

std::string simple_device::address() {
    boost::recursive_mutex::scoped_lock lock(state_mutex_);
    return address_;
}

std::string simple_device::label() {
    boost::recursive_mutex::scoped_lock lock(state_mutex_);
    return label_;
}

database_id simple_device::id() {
    boost::recursive_mutex::scoped_lock lock(state_mutex_);
    return id_;
}

void simple_device::on_updated() {
}

void simple_device::update(
    const std::string& new_address,
    const std::string& new_label)
{
    {
        boost::recursive_mutex::scoped_lock lock(state_mutex_);
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
        boost::recursive_mutex::scoped_lock lock(state_mutex_);
        address_ = new_address;
        label_ = new_label;
    }

    set_groups(groups);
    this->on_updated();
}

void simple_device::set_groups(const std::vector<std::string>& groups) {
    {
        boost::recursive_mutex::scoped_lock lock(state_mutex_);
        groups_.assign(groups.begin(), groups.end());
    }

    this->on_updated();
}

boost::recursive_mutex& simple_device::state_mutex() {
    return state_mutex_;
}