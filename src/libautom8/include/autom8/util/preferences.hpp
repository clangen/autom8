#pragma once

#include <string>
#include <sqlite/sqlite3.h>
#include <boost/lexical_cast.hpp>

namespace autom8 {
    class preferences {
    public:
        preferences(const std::string& name);
        ~preferences();

        template <typename KT, typename VT>
        VT& get(const KT& key, VT& value);

        template <typename KT, typename VT>
        bool set(const KT& key, const VT& value);

    private:
        sqlite3* connection_;
        std::string name_;
    };

    template <typename KT, typename VT>
    VT& preferences::get(const KT& key, VT& value) {
        std::string query = std::string(" SELECT value FROM ") + name_ + std::string(" WHERE key LIKE ?;");

        std::string key_str = boost::lexical_cast<std::string>(key);

        sqlite3_stmt* stmt = NULL;

        int result = sqlite3_prepare(
            connection_,
            query.c_str(),
            (int) query.size(),
            &stmt,
            NULL
        );

        if (result == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, key_str.c_str(), -1, SQLITE_STATIC);
            result = sqlite3_step(stmt);

            if (result == SQLITE_ROW) {
                std::string result_str((const char *) sqlite3_column_text(stmt, 0));

                try {
                    value = boost::lexical_cast<VT>(result_str);
                }
                catch (boost::bad_lexical_cast) {
                    // swallow it
                }
            }
        }

        sqlite3_finalize(stmt);

        return value;
    }

    template <typename KT, typename VT>
    bool preferences::set(const KT& key, const VT& value) {
        std::string key_str, value_str;
        try {
            key_str = boost::lexical_cast<std::string>(key);
            value_str = boost::lexical_cast<std::string>(value);
        }
        catch (boost::bad_lexical_cast) {
            return false;
        }

        std::string query = std::string(" REPLACE INTO ") + name_ + std::string(" (key, value) VALUES(?, ?);");

        sqlite3_stmt* stmt = NULL;

        int result = sqlite3_prepare(
            connection_,
            query.c_str(),
            (int) query.size(),
            &stmt,
            NULL);

        if (result == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, key_str.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, value_str.c_str(), -1, SQLITE_STATIC);
            result = sqlite3_step(stmt);
        }

        sqlite3_finalize(stmt);

        return (result == SQLITE_DONE);
    }
}
