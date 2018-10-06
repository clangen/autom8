#ifndef __C_AUTOM8_SSL_CERTIFICATE_HPP__
#define __C_AUTOM8_SSL_CERTIFICATE_HPP__

#include <string>

namespace autom8 {
    namespace ssl_certificate {
        bool exists();
        bool generate();
        bool remove();
        std::string fingerprint();
        std::string filename();
    }
}

#endif