//
// Created by RobinQu on 3/22/24.
//

#ifndef INSTINCT_MULTILINEGENERATIONOUTPUTPARSER_HPP
#define INSTINCT_MULTILINEGENERATIONOUTPUTPARSER_HPP

#include <google/protobuf/util/json_util.h>
#include "LLMGlobals.hpp"
#include "tools/Assertions.hpp"
#include "output_parser/BaseOutputParser.hpp"
#include "prompt/MessageUtils.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class MultilineGenerationOutputParser final: public BaseOutputParser<MultilineGeneration> {
    public:
        explicit MultilineGenerationOutputParser(const OutputParserOptions &options = {})
            : BaseOutputParser<MultilineGeneration>(options) {
        }

        MultilineGeneration ParseResult(const Generation &generation) override {
            MultilineGeneration multiline;
            const auto text = MessageUtils::StringifyGeneration(generation);
            auto lines = StringUtils::ReSplit(text, std::regex {"\n"})
                | std::views::transform(StringUtils::Trim)
                | std::views::filter(StringUtils::IsNotBlankString);
            for(auto line: lines) {
                *multiline.mutable_lines()->Add() = line;
            }
            return multiline;
        }

        std::string GetFormatInstruction() override {
            return "Please reply in a list seperated with new line separator strictly without any leading number items.";
        }

    };

    static OutputParserPtr<MultilineGeneration> CreateMultilineGenerationOutputParser(const OutputParserOptions &options = {}) {
        return std::make_shared<MultilineGenerationOutputParser>(options);
    }

}

#endif //INSTINCT_MULTILINEGENERATIONOUTPUTPARSER_HPP
