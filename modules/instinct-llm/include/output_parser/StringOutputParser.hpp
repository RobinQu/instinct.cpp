//
// Created by RobinQu on 3/23/24.
//

#ifndef INSTINCT_STRINGOUTPUTPARSER_HPP
#define INSTINCT_STRINGOUTPUTPARSER_HPP


#include "BaseOutputParser.hpp"

namespace INSTINCT_LLM_NS {
    class StringOutputParser final: public BaseOutputParser<std::string> {

    public:
        std::string ParseResult(const Generation &generation) override {
            if (generation.has_message()) {
                return generation.message().content();
            }
            return generation.text();
        }

        std::string GetFormatInstruction() override {
            return "";
        }
    };

    static OutputParserPtr<std::string> CreateStringOutputParser() {
        return std::make_shared<StringOutputParser>();
    }
}

// namespace xn::output {
//     static INSTINCT_LLM_NS::OutputParserPtr<std::string> string_output_parser {
//         return INSTINCT_LLM_NS::CreateStringOutputParser();
//     };
// }

#endif //INSTINCT_STRINGOUTPUTPARSER_HPP
