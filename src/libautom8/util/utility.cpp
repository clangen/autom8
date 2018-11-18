#include <autom8/util/utility.hpp>
#include <boost/filesystem.hpp>
#include <openssl/sha.h>
#include <sstream>

#if defined(WIN32)
#include <windows.h>
#endif

#include <utf8/utf8.h>

std::string get_environment_path(std::string varname) {
#if defined(WIN32)
    using namespace autom8::utility;
    std::wstring utf16_varname = u8to16(varname);

    wchar_t pwdir[4096];
    GetEnvironmentVariable(utf16_varname.c_str(), pwdir, sizeof(pwdir));

    return u16to8(std::wstring(pwdir));
#else
    return std::string(getenv("HOME"));
#endif
}

bool create_directory(std::string dir) {
    auto normalized = boost::filesystem::path(dir);
    boost::system::error_code ec;

    if (boost::filesystem::exists(normalized, ec)) {
        return true;
    }

    return boost::filesystem::create_directories(normalized, ec);
}

namespace autom8 {
    namespace utility {
        inline std::string u16to8(const std::wstring& u16) {
            std::string result;
            utf8::utf16to8(u16.begin(), u16.end(), std::back_inserter(result));
            return result;
        }

        inline std::wstring u8to16(const std::string& u8) {
            std::wstring result;
            utf8::utf8to16(u8.begin(), u8.end(), std::back_inserter(result));
            return result;
        }

        std::string home_directory() {
#if defined(WIN32)
            return get_environment_path("USERPROFILE");
#else
            return get_environment_path("HOME");
#endif
        }

        std::string settings_directory() {
            std::string settings_dir;
#if defined(WIN32)
            settings_dir = get_environment_path("APPDATA") + "\\autom8\\";
#else
            settings_dir = get_environment_path("HOME") + "/.autom8/";
#endif
            create_directory(settings_dir);
            return settings_dir;
        }

        std::string sha256(const char* data, unsigned int len) {
            unsigned char output[SHA256_DIGEST_LENGTH];
            memset(output, 0, SHA256_DIGEST_LENGTH);
            SHA256_CTX ctx = { 0 };
            SHA256_Init(&ctx);
            SHA256_Update(&ctx, (const void *) data, len);
            SHA256_Final((unsigned char *) output, &ctx);

            std::stringstream md5s_stream;
            for (unsigned int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                md5s_stream.fill('0');
                md5s_stream.width(2);
                md5s_stream << std::hex << (int) output[i];
            }

            return md5s_stream.str();
        }
    }
}
