//
// Created by RobinQu on 2024/3/10.
//

#ifndef STRINGOUTPUTPARSER_HPP
#define STRINGOUTPUTPARSER_HPP

#include <google/protobuf/util/json_util.h>
#include "IOutputParser.hpp"
#include "LLMGlobals.hpp"
#include "tools/Assertions.hpp"
#include "output_parser/BaseOutputParser.hpp"
#include "prompt/MessageUtils.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class StringOutputParser final: public BaseOutputParser<std::string> {
    public:
        std::string ParseResult(const Generation& result) override {
            return result.has_message() ? MessageUtils::FormatMessage(result.message()) : result.text();
        }

        std::string GetFormatInstruction() override {
            // tend to instruct nothing
            return "Please reply in plain text format.";
        }

    };

}

#endif //STRINGOUTPUTPARSER_HPP
