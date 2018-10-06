#ifndef __C_AUTOM8_UTILITY_HPP__
#define __C_AUTOM8_UTILITY_HPP__

#include <string>
#include <autom8/util/preferences.hpp>

namespace autom8 {
    namespace utility {
        std::string home_directory();
        std::string settings_directory();
        preferences& prefs();
        std::string u16to8(const std::wstring& u16);
        std::wstring u8to16(const std::string& u8);
        std::string sha256(const char* data, unsigned int len);
    }
}

#endif