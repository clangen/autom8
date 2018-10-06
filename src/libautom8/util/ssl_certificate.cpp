#include <autom8/util/ssl_certificate.hpp>
#include <autom8/util/utility.hpp>
#include <autom8/util/debug.hpp>

#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/engine.h>
#include <openssl/md5.h>

#include <boost/filesystem.hpp>

static const std::string TAG = "ssl_certificate";

using namespace autom8;

#include <utf8/utf8.h>

namespace autom8 {
    namespace ssl_certificate {
        bool exists() {
            return boost::filesystem::exists(
                boost::filesystem::path(filename()));
        }

        bool remove() {
            if ( ! exists()) {
                return true;
            }

            return boost::filesystem::remove(
                boost::filesystem::path(filename()));
        }

        std::string filename() {
            return utility::settings_directory() + "autom8_ssl.pem";
        }

        std::string rsa_md5(BIGNUM* pubkey_bignum) {
            char* pubkey_bytes = BN_bn2hex(pubkey_bignum);
            size_t pubkey_size = strlen(pubkey_bytes);

            if ( ! pubkey_size) {
                debug::log(debug::error, TAG, "*** fatal: rsa key size mismatch??");
                throw std::exception();
            }

            unsigned char pubkey_md5_bytes[MD5_DIGEST_LENGTH];

            MD5_CTX md5 = { 0 };
            MD5_Init(&md5);
            MD5_Update(&md5, pubkey_bytes, pubkey_size);
            MD5_Final(pubkey_md5_bytes, &md5);

            std::stringstream md5s_stream;
            for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
                md5s_stream.fill('0');
                md5s_stream.width(2);
                md5s_stream << std::hex << int(pubkey_md5_bytes[i]);
                if (i < (MD5_DIGEST_LENGTH - 1)) md5s_stream << ':';
            }

            delete pubkey_bytes;

            return md5s_stream.str();
        }

        std::string fingerprint() {
            std::string result;
            utility::prefs().get("fingerprint", result);

            if ( ! result.size()) {
                return "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
            }

            return result;
        }

        bool generate() {
            static const int days = 365 * 10;

            CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);

            bool result = true;
            FILE* outfile = NULL;
            X509* x509 = X509_new();
            EVP_PKEY* key = EVP_PKEY_new();
            RSA *rsa = RSA_generate_key(1024, RSA_F4, NULL, NULL);

            // store
            if ( ! rsa->n) {
                debug::log(debug::error, TAG, "*** fatal: rsa key has no public modulus??");
                throw std::exception();
            }

            std::string md5 = rsa_md5((BIGNUM *) rsa->n);
            utility::prefs().set("fingerprint", md5);

            if ( ! EVP_PKEY_assign_RSA(key, rsa)) {
                result = false;
            }

            X509_set_version(x509, 2);
            ASN1_INTEGER_set(X509_get_serialNumber(x509), 0);
            X509_gmtime_adj(X509_get_notBefore(x509), 0);
            X509_gmtime_adj(X509_get_notAfter(x509), (long) (60*60*24*days));
            X509_set_pubkey(x509, key);

            X509_NAME* name = X509_get_subject_name(x509);
            X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char*) "US", -1, -1, 0);
            X509_NAME_add_entry_by_txt(name,"CN", MBSTRING_ASC, (unsigned char*) "autom8", -1, -1, 0);

            X509_set_issuer_name(x509, name);

            if ( ! X509_sign(x509, key,EVP_md5())) {
                result = false;
            }
            else {
#if defined(WIN32)
                std::string utf8fn = filename();
                std::wstring utf16fn;
                utf8::utf8to16(utf8fn.begin(), utf8fn.end(), back_inserter(utf16fn));

                ::_wfopen_s(&outfile, utf16fn.c_str(), L"w+");
#else
                outfile = fopen(filename().c_str(), "w+");
#endif

                if (outfile) {
                    PEM_write_PrivateKey(outfile, key, NULL, NULL, 0, NULL, NULL);
                    PEM_write_X509(outfile, x509);
                    fflush(outfile);
                    fclose(outfile);
                }
            }

            X509_free(x509);
            EVP_PKEY_free(key);

            ENGINE_cleanup();
            CRYPTO_cleanup_all_ex_data();

            return result;
        }
    }
}

