#ifndef __C_AUTOM8_MESSAGE_MATCHER_HPP__
#define __C_AUTOM8_MESSAGE_MATCHER_HPP__

#include <autom8/util/json.hpp>
#include <boost/asio.hpp>

namespace autom8 {
    class message_matcher {
    public:
        typedef boost::true_type result_type;

        message_matcher(size_t max_length = 10485760) // maximum 10 meg message size by default
        : max_length_(max_length)
        , length_(0) {
        }

        template <typename Iterator>
        std::pair<Iterator, bool> operator() (Iterator begin, Iterator end) {
            Iterator i = begin;
            for ( ; i != end; i++) {
                if (*i == 0) {
                    return std::make_pair(i, true);
                }
                else {
                    ++length_;
                    if (length_ >= max_length_) {
                        return std::make_pair(i, true);
                    }
                }
            }

            return std::make_pair(end, false);
        }

    private:
        size_t max_length_, length_;
    };
}

#endif