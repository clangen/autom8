#include <autom8/message/request_handler_registrar.hpp>
#include <autom8/message/request_handler_factory.hpp>
#include <autom8/message/requests/get_device_list.hpp>
#include <autom8/message/requests/get_security_alert_count.hpp>
#include <autom8/message/requests/send_device_command.hpp>

#include <boost/thread.hpp>

using namespace autom8;

static bool registered;
static boost::mutex registration_mutex_;

void request_handler_registrar::register_all() {
    boost::mutex::scoped_lock lock(registration_mutex_);

    if ( ! registered) {
        autom8::request_handler_factory_ptr factory = autom8::request_handler_factory::instance();
        factory->register_handler(autom8::get_device_list::create());
        factory->register_handler(autom8::send_device_command::create());
        factory->register_handler(autom8::get_security_alert_count::create());

        registered = true;
    }
}