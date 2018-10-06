#ifndef __C_AUTOM8_DEVICE_MODEL_HPP__
#define __C_AUTOM8_DEVICE_MODEL_HPP__

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <autom8/device/device_factory.hpp>
#include <sqlite/sqlite3.h>
#include <sigslot/sigslot.h>

namespace autom8 {
    class device_model {
    public:
        device_model(device_factory_ptr factory);
        virtual ~device_model();

        sigslot::signal1<device_ptr> device_added;
        sigslot::signal1<database_id> device_removed;
        sigslot::signal1<database_id> device_updated;

        virtual device_ptr add(
            device_type type,
            const std::string& address,
            const std::string& label,
            const std::vector<std::string>& groups);

        virtual bool remove(device_ptr device);
        virtual bool remove(database_id id);

        virtual bool update(device_ptr device);
        virtual bool update(
            database_id id,
            device_type type,
            const std::string& address,
            const std::string& label,
            const std::vector<std::string>& groups);

        virtual int all_devices(device_list& target);
        virtual device_ptr find_by_address(const std::string& address);

    protected:
        void create_tables();
        sqlite3* connection() { return connection_; }
        device_factory_ptr factory() { return factory_; }

        virtual void on_device_added(device_ptr new_device);
        virtual void on_device_removed(database_id old_device_id);
        virtual void on_device_updated(database_id id);

        bool remove_groups(database_id id);
        void get_groups(database_id id, std::vector<std::string>& groups);
        bool set_groups(database_id id, const std::vector<std::string>& groups);

    private:
        device_factory_ptr factory_;
        std::string device_table_name_, groups_table_name_;
        sqlite3* connection_;
        boost::mutex connection_mutex_;
    };

    typedef boost::shared_ptr<device_model> device_model_ptr;
}

#endif