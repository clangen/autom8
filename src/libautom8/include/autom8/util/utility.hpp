#pragma once

#include <string>

namespace autom8 {
    namespace utility {
        std::string home_directory();
        std::string settings_directory();
        std::string u16to8(const std::wstring& u16);
        std::wstring u8to16(const std::string& u8);
        std::string sha256(const char* data, unsigned int len);
    }
}
