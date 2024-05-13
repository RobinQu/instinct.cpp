//
// Created by RobinQu on 2024/5/13.
//

#ifndef LLMCOMPILERTHOUGHTOUTPUTPARSER_HPP
#define LLMCOMPILERTHOUGHTOUTPUTPARSER_HPP

#include "LLMGlobals.hpp"
#include "output_parser/BaseOutputParser.hpp"

namespace INSTINCT_LLM_NS {
    class LLMCompilerThoughtOutputParser final: public BaseOutputParser<AgentThought> {
    public:
        explicit LLMCompilerThoughtOutputParser(const OutputParserOptions &options)
            : BaseOutputParser<AgentThought>(options) {
        }

        AgentThought ParseResult(const Generation &context) override {

        }
    };

    static OutputParserPtr<AgentThought> CreateLLMCompilerOutputParser(const OutputParserOptions& options = {}) {
        return std::make_shared<LLMCompilerThoughtOutputParser>(options);
    }
}

#endif //LLMCOMPILERTHOUGHTOUTPUTPARSER_HPP
