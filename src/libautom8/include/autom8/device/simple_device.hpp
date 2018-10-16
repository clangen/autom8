#pragma once

#include <memory>
#include <mutex>
#include <autom8/device/device_base.hpp>

namespace autom8 {
    class simple_device: public device_base {
    public:
        simple_device(
            database_id id,
            const std::string& address,
            const std::string& label,
            const std::vector<std::string>& groups = std::vector<std::string>()
        );

        virtual ~simple_device();

        virtual device_status status();
        virtual std::string address();
        virtual std::string label();
        virtual database_id id();
        virtual void groups(std::vector<std::string>& target);

        virtual device_type type() = 0;
        virtual void turn_on() = 0;
        virtual void turn_off() = 0;

        virtual void update(
            const std::string& new_address,
            const std::string& new_label);

        virtual void update(
            const std::string& new_address,
            const std::string& new_label,
            const std::vector<std::string>& groups);

        virtual void set_groups(const std::vector<std::string>& groups);

    protected:
        virtual void on_updated();
        std::recursive_mutex& state_mutex();
        std::unique_lock<std::recursive_mutex> state_lock();

    protected:
        std::string label_;
        std::string address_;
        std::vector<std::string> groups_;
        device_status status_;
        database_id id_;
        std::recursive_mutex state_mutex_;
    };
}
