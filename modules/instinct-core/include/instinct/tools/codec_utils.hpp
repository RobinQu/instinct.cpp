//
// Created by RobinQu on 2024/3/18.
//

#ifndef INSTINCT_CODECUTILS_HPP
#define INSTINCT_CODECUTILS_HPP

#include <libbase64.h>

#include <instinct/core_global.hpp>
#include <instinct/exception/instinct_exception.hpp>

namespace INSTINCT_CORE_NS {
    class CodecUtils final {
    public:

        /**
         * detailed usage for decoding: https://github.com/aklomp/base64/blob/master/test/test_base64.c#L72
         * @param encoded
         * @return
         */
        static std::string DecodeBase64(const std::string& encoded) {
            static char base_64_out[2000];
            static size_t base_64_out_len;
            if(!base64_decode(encoded.data(), encoded.size(), base_64_out, &base_64_out_len, 0)) {
                throw InstinctException("base64 decode error");
            }
            return {base_64_out, base_64_out_len};
        }

        /**
         * detailed usage for encoding: https://github.com/aklomp/base64/blob/master/test/test_base64.c#L18C2-L18C50
         * @param buf
         * @return
         */
        static std::string EncodeBase64(const std::string& buf) {
            static char base_64_out[2000];
            static size_t base_64_out_len;
            base64_encode(buf.data(), buf.size(), base_64_out, &base_64_out_len, 0);
            return  {base_64_out, base_64_out_len};
        }
    };
}

#endif //INSTINCT_CODECUTILS_HPP
