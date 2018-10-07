#pragma once

#include <sigslot/sigslot.h>
#include <string>

namespace autom8 {
    class debug {
    public:
        enum debug_level {
            info = 0,
            warning,
            error
        };

        static void init();
        static void deinit();
        static sigslot::signal3<debug_level, std::string, std::string> string_logged;
        static void log(debug_level level, const std::string& tag, const std::string& string);
    };
}
