//
// Created by RobinQu on 2024/3/12.
//

#ifndef MULTILINETEXTOUTPUTPARSER_HPP
#define MULTILINETEXTOUTPUTPARSER_HPP

#include <llm.pb.h>

#include "LLMGlobals.hpp"
#include "RetrievalGlobals.hpp"
#include "output_parser/BaseOutputParser.hpp"
#include <string_view>
#include <iomanip>
#include <ranges>


namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    class MultiLineTextOutputParser final: public BaseOutputParser<MultiLineText> {

        std::string instruction_content_;
    public:
        explicit MultiLineTextOutputParser(std::string  instruction_content = "Please provide answers seperated by new lines strictly."): instruction_content_(std::move(instruction_content)) {}

        std::vector<std::string> ParseResult(const Generation& result) override {
            static std::regex delimiter  {"\n+"};
            auto& content = result.has_message() ? result.message().content() : result.text();
            auto lines_view = StringUtils::ReSplit(content, delimiter)
                    | std::views::filter([](const auto& s) { return !StringUtils::IsBlankString(s); });
            return {lines_view.begin(), lines_view.end()};
        }

        std::string GetFormatInstruction() override {
            return instruction_content_;
        }
    };
}

#endif //MULTILINETEXTOUTPUTPARSER_HPP
