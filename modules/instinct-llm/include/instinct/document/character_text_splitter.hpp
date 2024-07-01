//
// Created by RobinQu on 2024/3/5.
//

#ifndef CHARACTERTEXTSPLITTER_HPP
#define CHARACTERTEXTSPLITTER_HPP

#include <instinct/document/base_text_splitter.hpp>

namespace INSTINCT_LLM_NS {

    struct CharacterTextSplitterOptions {
        LengthCalculatorPtr length_function = std::make_shared<StringLengthCalculator>();
        int chunk_size=4000;
        int chunk_overlap=200;
        bool keep_separator=false;
        bool strip_whitespace=true;
        UnicodeString separator = "\n\n";
    };

    class CharacterTextSplitter final: public BaseTextSplitter {
        UnicodeString separator_;
    public:
        explicit CharacterTextSplitter(const CharacterTextSplitterOptions& options = {}): BaseTextSplitter(options.chunk_size, options.chunk_overlap, options.keep_separator, options.strip_whitespace, options.length_function), separator_(options.separator) {}

        std::vector<UnicodeString> SplitText(const UnicodeString& text) override {
            auto sep = details::escape_for_regular_expression(separator_);
            std::vector<UnicodeString> splits = details::split_text_with_seperator(text, sep, keep_separator_);
            std::vector<UnicodeString> results;
            MergeSplits_(splits, sep, results);
            return results;
        }

    };
}


#endif //CHARACTERTEXTSPLITTER_HPP
