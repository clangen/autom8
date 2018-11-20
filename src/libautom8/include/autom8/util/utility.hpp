#pragma once

#include <string>

namespace autom8 {
    namespace utility {
        std::string sha256(const char* data, unsigned int len);
        std::string sha256(const std::string& data);
    }
}
