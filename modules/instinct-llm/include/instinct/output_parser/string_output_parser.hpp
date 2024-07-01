//
// Created by RobinQu on 3/23/24.
//

#ifndef INSTINCT_STRINGOUTPUTPARSER_HPP
#define INSTINCT_STRINGOUTPUTPARSER_HPP


#include <instinct/output_parser/base_output_parser.hpp>

namespace INSTINCT_LLM_NS {
    class StringOutputParser final: public BaseOutputParser<std::string> {

    public:
        explicit StringOutputParser(const OutputParserOptions &options = {})
            : BaseOutputParser<std::string>(options) {
        }

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

    static OutputParserPtr<std::string> CreateStringOutputParser(const OutputParserOptions& options = {}) {
        return std::make_shared<StringOutputParser>(options);
    }
}


#endif //INSTINCT_STRINGOUTPUTPARSER_HPP
