#include <autom8/db/db.hpp>

#define EXEC_SQL(connection, str)               \
    {                                           \
        sqlite3_stmt* stmt = NULL;              \
        std::string query(str);                 \
                                                \
        int result = sqlite3_prepare_v2(        \
            connection,                         \
            query.c_str(),                      \
            (int) query.size(),                 \
            &stmt,                              \
            NULL);                              \
                                                \
        if (result == SQLITE_OK) {              \
            result = sqlite3_step(stmt);        \
        }                                       \
                                                \
        sqlite3_finalize(stmt);                 \
    }                                           \

namespace autom8 { namespace db {
    /*~*~*~*~ transaction begin ~*~*~*~*/
    transaction::transaction(sqlite3* sqlite)
    : sqlite_(sqlite)
    , success_(false) {
        EXEC_SQL(sqlite_, "BEGIN TRANSACTION;");
    }

    void transaction::set_successful(bool success) {
        success_ = success;
    }

    transaction::~transaction() {
        if (success_) {
            EXEC_SQL(sqlite_, "COMMIT TRANSACTION;");
        }
        else {
            EXEC_SQL(sqlite_, "END TRANSACTION;");
        }
    }
    /*~*~*~*~ transaction end ~*~*~*~*/
    /*~*~*~*~ statement begin ~*~*~*~*/
    statement::statement(sqlite3* sqlite, const std::string& statement_value)
    : sqlite_(sqlite)
    , stmt_(NULL)
    , executed_(false)
    , done_(false) {
        int result = sqlite3_prepare(
            sqlite_,
            statement_value.c_str(),
            (int) statement_value.size(),
            &stmt_,
            NULL);

        if (result != SQLITE_OK) {
            throw std::exception(/*"could not create sqlite3 statement"*/);
        }
    }

    statement::~statement() {
        if (stmt_) {
            sqlite3_finalize(stmt_);
            stmt_ = NULL;
        }
    }

    void statement::bind_int(int column, int value) {
        sqlite3_bind_int(stmt_, column, value);
    }

    void statement::bind_int64(int column, sqlite3_int64 value) {
        sqlite3_bind_int64(stmt_, column, value);
    }

    void statement::bind_string(int column, const std::string& string) {
        sqlite3_bind_text(stmt_, column, string.c_str(), -1, SQLITE_STATIC);
    }

    bool statement::execute() {
        if (executed_) {
            throw std::exception(/*"statement already executed"*/);
        }

        int result = sqlite3_step(stmt_);
        executed_ = true;

        return (result == SQLITE_OK || result == SQLITE_DONE);
    }

    bool statement::has_next() {
        if (executed_) {
            return false;
        }

        return !done_;
    }

    bool statement::next() {
        if (!has_next() || executed_) {
            return false;
        }

        int result = sqlite3_step(stmt_);

        if (result == SQLITE_DONE) {
            done_ = true;
            return false;
        }
        else if (result == SQLITE_ROW) {
            return true;
        }
        else {
            throw std::exception(/*"sqlite3_step failed"*/);
        }
    }

    int statement::get_int(int column) {
        return sqlite3_column_int(stmt_, column);
    }

    sqlite3_int64 statement::get_int64(int column) {
        return sqlite3_column_int64(stmt_, column);
    }

    std::string statement::get_string(int column) {
        const char* text = (const char*) sqlite3_column_text(stmt_, column);
        return text ? std::string(text) : std::string();
    }
    /*~*~*~*~ statement end~*~*~*~*/
} }