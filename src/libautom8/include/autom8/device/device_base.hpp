#ifndef __C_AUTOM8_DEVICE_HPP__
#define __C_AUTOM8_DEVICE_HPP__

#include <autom8/constants.h>
#include <autom8/util/json.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <sigslot/sigslot.h>

#include <vector>
#include <map>

namespace autom8 {
    //////////////////////////////////////////
    //      base class for all devices      //
    //////////////////////////////////////////
    class device_base;
    typedef boost::shared_ptr<device_base> device_ptr;

    class device_base: public boost::enable_shared_from_this<device_base> {
    public:
        sigslot::signal1<device_ptr> status_changed;

        virtual ~device_base() { }

        virtual void turn_on() = 0;
        virtual void turn_off() = 0;
        virtual device_status status() = 0;
        virtual std::string address() = 0;
        virtual void groups(std::vector<std::string>& target) = 0;
        virtual std::string label() = 0;
        virtual device_type type() = 0;
        virtual database_id id() = 0;

        template <typename T>
        T* get_interface(T*& target);

        virtual json_value_ref to_json();

    protected:
        virtual void on_status_changed();
        virtual void get_extended_json_attributes(json_value& target);
    };

    template <typename T>
    T* device_base::get_interface(T*& target) {
        target = dynamic_cast<T*>(this);
        return target;
    }

    typedef std::vector<device_ptr> device_list;

    //////////////////////////////////////////
    //   device specialization interfaces   //
    //////////////////////////////////////////

    class lamp {
    public:
        virtual int brightness() = 0;
        virtual void set_brightness(int brightness) = 0;
    };

    class security_sensor {
    public:
        virtual void arm() = 0;
        virtual void disarm() = 0;
        virtual void reset() = 0;
        virtual bool is_armed() = 0;
        virtual bool is_tripped() = 0;
    };
}

#endif