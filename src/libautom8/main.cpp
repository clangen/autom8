#include <iostream>

#include <autom8/util/signal_handler.hpp>
#include <autom8/constants.h>

#include <autom8/message/message.hpp>
#include <autom8/message/response.hpp>
#include <autom8/message/request_handler_factory.hpp>
#include <autom8/message/request_handler_registrar.hpp>

#include <autom8/net/server.hpp>
#include <autom8/net/session.hpp>
#include <autom8/net/server.hpp>

#include <autom8/util/debug.hpp>
#include <autom8/util/utility.hpp>

#include <autom8/autom8.hpp>
#include <autom8/device/x10/cm15a/cm15a_device_system.hpp>

#include <boost/date_time.hpp>

class debug_logger: public autom8::signal_handler {
public:
    debug_logger() {
        autom8::debug::string_logged.connect(this, &debug_logger::on_string_logged);
    }

    void on_string_logged(autom8::debug::debug_level, std::string tag, std::string string) {
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        time_facet* facet(new time_facet("[%x %X] "));
        std::cout.imbue(std::locale(std::cout.getloc(), facet));
        ptime time(microsec_clock::local_time());
        std::cout << time << string << std::endl;
    }
};

int main() {
    using namespace autom8;

    debug_logger log;

    int port = 7901;
    utility::prefs().get("port", port);

    device_system_ptr cm15a(new autom8::cm15a_device_system());
    device_system::set_instance(cm15a);

    server::start(port);

    device_system::clear_instance();
}