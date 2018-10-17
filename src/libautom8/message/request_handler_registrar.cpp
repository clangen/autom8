#include <autom8/message/request_handler_registrar.hpp>
#include <autom8/message/request_handler_factory.hpp>
#include <autom8/message/requests/get_device_list.hpp>
#include <autom8/message/requests/get_security_alert_count.hpp>
#include <autom8/message/requests/send_device_command.hpp>
#include <mutex>

using namespace autom8;

static bool registered;
static std::mutex registration_mutex_;

void request_handler_registrar::register_all() {
    std::unique_lock<decltype(registration_mutex_)> lock(registration_mutex_);

    if (!registered) {
        autom8::request_handler_factory::ptr factory = autom8::request_handler_factory::instance();
        factory->register_handler(autom8::get_device_list::create());
        factory->register_handler(autom8::send_device_command::create());
        factory->register_handler(autom8::get_security_alert_count::create());

        registered = true;
    }
}