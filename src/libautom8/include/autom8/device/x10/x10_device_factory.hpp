#ifndef __C_AUTOM8_X10_DEVICE_FACTORY_HPP__
#define __C_AUTOM8_X10_DEVICE_FACTORY_HPP__

#include <autom8/device/device_factory.hpp>
#include <boost/thread.hpp>
#include <map>

namespace autom8 {
    class x10_device_system;

    class x10_device_factory: public device_factory {
    private:
        typedef std::map<database_id, device_ptr> id_device_map;

    public:
        x10_device_factory(x10_device_system* owner);

        virtual device_ptr create(
            database_id id,
            device_type type,
            const std::string& address,
            const std::string& label,
            const std::vector<std::string>& groups);

        void device_removed(database_id id);
        void device_updated(database_id id);

        virtual std::string name() const;

    private:
        x10_device_system* owner_;
        id_device_map id_device_map_;
        boost::mutex id_device_map_mutex_;
    };
}

#endif