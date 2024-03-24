//
// Created by RobinQu on 3/23/24.
//

#ifndef INSTINCT_STRINGOUTPUTPARSER_HPP
#define INSTINCT_STRINGOUTPUTPARSER_HPP


#include "BaseOutputParser.hpp"

namespace INSTINCT_LLM_NS {
    class StringOutputParser: public BaseOutputParser<std::string> {
    public:
        explicit StringOutputParser(const OutputParserOptions &options = {}) : BaseOutputParser(options) {}

    public:
        std::string ParseResult(const JSONContextPtr &context) override {
            auto generation = context->RequireMessage<Generation>(
                    GetOptions().generation_input_key
            );
            if (generation.has_message()) {
                return generation.message().content();
            }
            return generation.text();
        }

        std::string GetFormatInstruction() override {
            return "";
        }

    };
}

#endif //INSTINCT_STRINGOUTPUTPARSER_HPP
