//
// Created by RobinQu on 2024/3/10.
//

#ifndef STRINGOUTPUTPARSER_HPP
#define STRINGOUTPUTPARSER_HPP

#include <google/protobuf/util/json_util.h>
#include "IOutputParser.hpp"
#include "LLMGlobals.hpp"
#include "tools/Assertions.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class StringOutputParser final: public IOutputParser<std::string> {
    public:
        std::string ParseResult(const std::string& result) override {
            return result;
        }
    };

}

#endif //STRINGOUTPUTPARSER_HPP
