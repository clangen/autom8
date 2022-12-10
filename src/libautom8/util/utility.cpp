#include <autom8/util/utility.hpp>
#include <openssl/sha.h>
#include <sstream>
#include <string.h>

namespace autom8 {
    namespace utility {
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

        std::string sha256(const std::string& data) {
            return sha256(data.c_str(), data.size());
        }
    }
}
