#ifndef __C_AUTOM8_DB_HPP__
#define __C_AUTOM8_DB_HPP__

#include <sqlite/sqlite3.h>
#include <string>

namespace autom8 {
    namespace db {
        class transaction {
        public:
            transaction(sqlite3* sqlite);
            virtual ~transaction();
            void set_successful(bool success);

        private:
            sqlite3* sqlite_;
            bool success_;
        };

        class statement {
        public:
            statement(sqlite3* sqlite, const std::string& statement_value);
            virtual ~statement();
            void bind_int(int column, int value);
            void bind_int64(int column, sqlite3_int64 value);
            void bind_string(int column, const std::string& string);

            bool has_next();
            bool next();

            bool execute();

            int get_int(int column);
            sqlite3_int64 get_int64(int column);
            std::string get_string(int column);

        private:
            sqlite3* sqlite_;
            sqlite3_stmt* stmt_;
            bool executed_, done_;
        };
    }
}

#endif

