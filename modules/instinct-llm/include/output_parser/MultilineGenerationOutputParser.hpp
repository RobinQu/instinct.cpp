//
// Created by RobinQu on 3/22/24.
//

#ifndef INSTINCT_MULTILINEGENERATIONOUTPUTPARSER_HPP
#define INSTINCT_MULTILINEGENERATIONOUTPUTPARSER_HPP

#include <google/protobuf/util/json_util.h>
#include "IOutputParser.hpp"
#include "LLMGlobals.hpp"
#include "tools/Assertions.hpp"
#include "output_parser/BaseOutputParser.hpp"
#include "prompt/MessageUtils.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class MultilineGenerationOutputParse final: public BaseOutputParser<MultilineGeneration> {

    public:
        explicit MultilineGenerationOutputParse(const OutputParserOptions &options) : BaseOutputParser<MultilineGeneration>(options) {}


        MultilineGeneration ParseResult(const JSONContextPtr &context) override {
//            return
            MultilineGeneration multiline;
            return multiline;
        }

        std::string GetFormatInstruction() override {
            // TODO give instruction
            return "";
        }

    };

}

#endif //INSTINCT_MULTILINEGENERATIONOUTPUTPARSER_HPP
