#pragma once

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
