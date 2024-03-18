//
// Created by vscode on 3/18/24.
//

#ifndef INSTINCT_CODECUTILS_HPP
#define INSTINCT_CODECUTILS_HPP

#include <string>
#include <libbase64.h>

#include "../CoreGlobals.hpp"


namespace INSTINCT_CORE_NS {
    class CodecUtils final {
        static std::string DecodeBase64String(const std::string& input) {
//            if(base64_decode(splits[0].data(), splits[0].size(), base_64_out, &base_64_out_len, 0)) {
//                throw InstinctException("base64 decode error");
//            }
        }
    };
}


#endif //INSTINCT_CODECUTILS_HPP
