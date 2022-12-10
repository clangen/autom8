#include <autom8/device/device_model.hpp>
#include <autom8/util/utility.hpp>
#include <autom8/db/db.hpp>

#include <f8n/str/util.h>
#include <f8n/debug/debug.h>
#include <f8n/environment/Environment.h>

using namespace autom8;
using debug = f8n::debug;

/* table names are prefixed by factory type */
#define DEVICE_TABLE_SUFFIX "_device"
#define GROUPS_TABLE_SUFFIX "_groups"

#define ID_COLUMN "id"
#define ADDRESS_COLUMN "address"
#define TYPE_COLUMN "type"
#define LABEL_COLUMN "label"
#define GROUP_NAME_COLUMN "name"
#define DEVICE_ID_COLUMN "device_id"

static const std::string TAG = "device_model";
static const int OPEN_FLAGS = SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX;

device_model::device_model(device_factory_ptr factory)
: factory_(factory)
, connection_(NULL) {
    device_table_name_ = factory_->name() + DEVICE_TABLE_SUFFIX;
    groups_table_name_ = factory_->name() + GROUPS_TABLE_SUFFIX;

    // connect to the db.
    std::string filename = f8n::env::GetDataDirectory() + "devices.db";
    if (sqlite3_open_v2(filename.c_str(), &connection_, OPEN_FLAGS, 0) != SQLITE_OK) {
        debug::error(TAG, "unable to open devices database!");
        throw std::exception();
    }

    create_tables();
}

device_model::~device_model() {
    if (connection_) {
        sqlite3_close(connection_);
        connection_ = NULL;
    }
}

void device_model::create_tables() {
    /* devices table */
    {
        std::string create_table = f8n::str::format(
            " CREATE TABLE IF NOT EXISTS %s ("
            " %s INTEGER PRIMARY KEY AUTOINCREMENT, "
            " %s INTEGER, "
            " %s STRING UNIQUE, "
            " %s STRING); ",
            device_table_name_.c_str(),
            ID_COLUMN,
            TYPE_COLUMN,
            ADDRESS_COLUMN,
            LABEL_COLUMN);

        autom8::db::statement stmt(connection_, create_table);
        if (!stmt.execute()) {
            debug::error(TAG, "unable to create devices table");
            throw std::exception();
        }
    }

    /* groups table */
    {
        std::string create_table = f8n::str::format(
            " CREATE TABLE IF NOT EXISTS %s ("
            " %s INTEGER PRIMARY KEY AUTOINCREMENT, "
            " %s STRING, "
            " %s INTEGER); ",
            groups_table_name_.c_str(),
            ID_COLUMN,
            GROUP_NAME_COLUMN,
            DEVICE_ID_COLUMN);

        autom8::db::statement stmt(connection_, create_table);
        if (!stmt.execute()) {
            debug::error(TAG, "unable to create devices table");
            throw std::exception();
        }
    }
}

device_ptr device_model::add(
    device_type type,
    const std::string& address,
    const std::string& label,
    const std::vector<std::string>& groups)
{
    device_ptr device;

    std::string insert_row = f8n::str::format(
        " INSERT INTO %s (%s, %s, %s, %s)"
        " VALUES(NULL, ?, ?, ?);",
        device_table_name_.c_str(),
        ID_COLUMN,
        TYPE_COLUMN,
        ADDRESS_COLUMN,
        LABEL_COLUMN);

    {
        std::unique_lock<decltype(connection_mutex_)> lock(connection_mutex_);

        autom8::db::statement stmt(connection_, insert_row);
        stmt.bind_int(1, type);
        stmt.bind_string(2, address);
        stmt.bind_string(3, label);

        if (stmt.execute()) {
            database_id row_id = sqlite3_last_insert_rowid(connection_);
            set_groups(row_id, groups);

            device = factory_->create(row_id, type, address, label, groups);
        }
    }

    if (device) {
        on_device_added(device);
    }

    return device;
}

bool device_model::remove(device_ptr device) {
    return remove(device->id());
}

bool device_model::remove(database_id id) {
    bool result = false;

    std::string delete_device = f8n::str::format(
        " DELETE FROM %s"
        " WHERE %2%=?;",
        device_table_name_.c_str(),
        ID_COLUMN);

    {
        std::unique_lock<decltype(connection_mutex_)> lock(connection_mutex_);

        autom8::db::statement stmt(connection_, delete_device);
        stmt.bind_int64(1, id);

        result = stmt.execute();
        remove_groups(id);
    }

    if (result) {
        on_device_removed(id);
    }

    return result;
}

bool device_model::update(device_ptr device) {
    return this->update(
        device->id(),
        device->type(),
        device->address(),
        device->label(),
        device->groups());
}

bool device_model::update(
    database_id id,
    device_type type,
    const std::string& address,
    const std::string& label,
    const std::vector<std::string>& groups)
{
    std::string query = f8n::str::format(
        " UPDATE %1%"
        " SET %2%=?, %3%=?, %4%=?"
        " WHERE %5%=?;",
        device_table_name_.c_str(),
        TYPE_COLUMN,
        ADDRESS_COLUMN,
        LABEL_COLUMN,
        ID_COLUMN);

    bool result;

    {
        std::unique_lock<decltype(connection_mutex_)> lock(connection_mutex_);

        autom8::db::statement stmt(connection_, query);
        stmt.bind_int(1, type);
        stmt.bind_string(2, address);
        stmt.bind_string(3, label);
        stmt.bind_int64(4, id);

        result = stmt.execute();
        set_groups(id, groups);
    }

    if (result) {
        on_device_updated(id);
    }

    return result;
}

device_ptr device_model::find_by_address(const std::string& address_to_find) {
    device_ptr device;

    std::string query = f8n::str::format(
        " SELECT %s, %s, %s, %s"
        " FROM %s"
        " WHERE address=?;",
        ID_COLUMN,
        TYPE_COLUMN,
        ADDRESS_COLUMN,
        LABEL_COLUMN,
        device_table_name_.c_str());

    std::unique_lock<decltype(connection_mutex_)> lock(connection_mutex_);

    /* device itself */
    autom8::db::statement stmt(connection_, query);
    stmt.bind_string(1, address_to_find);

    if (stmt.next()) {
        database_id id = (database_id) stmt.get_int64(0);
        device_type type = (device_type) stmt.get_int(1);
        std::string address = stmt.get_string(2);
        std::string label = stmt.get_string(3);

        std::vector<std::string> groups;
        get_groups(id, groups);

        device = factory_->create(id, type, address, label, groups);
    }

    return device;
}

device_list device_model::all_devices() const {
    std::string query = f8n::str::format(
        " SELECT %1%, %2%, %3%, %4%"
        " FROM %5%"
        " ORDER BY %3%;",
        ID_COLUMN,
        TYPE_COLUMN,
        ADDRESS_COLUMN,
        LABEL_COLUMN,
        device_table_name_.c_str(),
        ADDRESS_COLUMN);

    autom8::db::statement stmt(connection_, query);

    device_list list;
    while (stmt.next()) {
        database_id id = (database_id) stmt.get_int64(0);
        device_type type = (device_type) stmt.get_int(1);
        std::string address = stmt.get_string(2);
        std::string label = stmt.get_string(3);

        std::vector<std::string> groups;
        get_groups(id, groups);

        auto created = factory_->create(id, type, address, label, groups);
        if (created) {
            list.push_back(created);
        }
        else {
            debug::w("device_model", "failed to create device");
        }
    }

    return list;
}

bool device_model::remove_groups(database_id id) {
    std::string query = f8n::str::format(
        " DELETE FROM %s"
        " WHERE %2%=?;",
        groups_table_name_.c_str(),
        DEVICE_ID_COLUMN);

    autom8::db::statement stmt(connection_, query);
    stmt.bind_int64(1, id);
    return stmt.execute();
}

bool device_model::set_groups(database_id id, const std::vector<std::string>& groups) {
    autom8::db::transaction transaction(connection_);

    bool ok = remove_groups(id);

    /* add */
    if (ok) {
        for (size_t i = 0; i < groups.size(); i++) {
            std::string query = f8n::str::format(
                " INSERT INTO %s (%s, %s, %s)"
                " VALUES(NULL, ?, ?);",
                groups_table_name_.c_str(),
                ID_COLUMN,
                GROUP_NAME_COLUMN,
                DEVICE_ID_COLUMN);

            autom8::db::statement stmt(connection_, query);
            stmt.bind_string(1, groups[i]);
            stmt.bind_int64(2, id);
            ok |= stmt.execute();
        }
    }

    transaction.set_successful(ok);

    return ok;
}

void device_model::get_groups(database_id id, std::vector<std::string>& groups) const {
    std::string query = f8n::str::format(
        " SELECT %s"
        " FROM %s"
        " WHERE %s=?;",
        GROUP_NAME_COLUMN,
        groups_table_name_.c_str(),
        DEVICE_ID_COLUMN);

    autom8::db::statement stmt(connection_, query);
    stmt.bind_int64(1, id);

    while (stmt.next()) {
        groups.push_back(stmt.get_string(0));
    }
}

void device_model::on_device_added(device_ptr new_device)
{
    device_added(new_device);
}

void device_model::on_device_removed(database_id old_device_id)
{
    device_removed(old_device_id);
}

void device_model::on_device_updated(database_id id)
{
    device_updated(id);
}
