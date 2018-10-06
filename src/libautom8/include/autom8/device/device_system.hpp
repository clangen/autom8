#ifndef __C_AUTOM8_DEVICE_SYSTEM_HPP__
#define __C_AUTOM8_DEVICE_SYSTEM_HPP__

#include <boost/shared_ptr.hpp>
#include <autom8/device/device_base.hpp>
#include <autom8/device/device_model.hpp>

namespace autom8 {
    class device_system;
    typedef boost::shared_ptr<device_system> device_system_ptr;

    class device_system {
    public:
        virtual std::string description() = 0;
        virtual device_model& model() = 0;

        static device_system_ptr instance();
        static device_system_ptr set_instance(device_system_ptr new_instance);
        static void clear_instance();

    protected:
        device_system() { }
        virtual ~device_system() { }
    };
}

#endif