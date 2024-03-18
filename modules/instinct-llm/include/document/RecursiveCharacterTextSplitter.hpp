//
// Created by RobinQu on 2024/2/28.
//

#ifndef RECURSIVECHARACTERTEXTSPLITTER_HPP
#define RECURSIVECHARACTERTEXTSPLITTER_HPP
#include "CoreGlobals.hpp"
#include <unicode/regex.h>
#include <unicode/ustring.h>

#include "BaseTextSplitter.hpp"
#include "LanguageSplitters.hpp"
#include "tokenizer/TiktokenTokenizer.hpp"
#include "tokenizer/Tokenizer.hpp"

namespace INSTINCT_LLM_NS {
    using namespace U_ICU_NAMESPACE;

    static std::vector<UnicodeString> DEFAULT_SEPARATOR_FOR_TEXT_SPLITTER = {
        "\n\n",
        "\n",
        " ",
        ""
    };

    struct RecursiveCharacterTextSplitterOptions {
        LengthFunction length_function = IdentityLengthFunction;
        int chunk_size=4000;
        bool keep_separator=true;
        bool strip_whitespace=true;
        std::vector<UnicodeString> separators = DEFAULT_SEPARATOR_FOR_TEXT_SPLITTER;
    };


    /**
     * \brief Recursively split text using a sequence of given characters as splitter. Copied a lot from Langchain Python.
     */
    class RecursiveCharacterTextSplitter: public BaseTextSplitter {
        std::vector<UnicodeString> separators_;
    public:
        explicit RecursiveCharacterTextSplitter(const RecursiveCharacterTextSplitterOptions& options = {}): BaseTextSplitter(
            options.chunk_size,
            0,
            options.keep_separator,
            options.strip_whitespace,
            options.length_function),
                                                                                                            separators_(options.separators) {

        }

        static RecursiveCharacterTextSplitter* FromTiktokenTokenizer(TiktokenTokenizer* tokenizer, RecursiveCharacterTextSplitterOptions options = {}) {
            options.length_function = [=](const UnicodeString& str) {
                return tokenizer->Encode(str).size();
            };
            return new RecursiveCharacterTextSplitter(options);
        }

        std::vector<UnicodeString> SplitText(const UnicodeString& text) override {
            auto seps = std::vector(separators_);
            std::vector<UnicodeString> result;
            SplitText_(text, seps, result);
            return result;
        }

    private:
        void SplitText_(const UnicodeString& text, std::vector<UnicodeString>& separators, std::vector<UnicodeString>& final_chunks) { // NOLINT(*-no-recursion)
            // default to last sep, assuming it's most common case in text
            UnicodeString separator = details::escape_for_regular_expression(separators.back());
            for(auto itr=separators.begin(); itr != separators.end(); ++itr) {
                auto sep = *itr;
                // break if it's empty string
                if(sep == "") {
                    separator = details::escape_for_regular_expression(sep);
                    separators.clear();
                    break;
                }
                // break if text can be split by sep
                if(text.indexOf(sep) > 0) {
                    separator = details::escape_for_regular_expression(sep);
                    // separators = std::vector(itr+1, separators.end());
                    separators.erase(separators.begin(), itr + 1);
                    break;
                }
            }

            const auto splits = details::split_text_with_seperator(text, separator, keep_sepeartor_);
            std::vector<UnicodeString> good_splits;
            const auto merging_separator = keep_sepeartor_ ? "" : separator;
            for(auto& s: splits) {
                if(length_function_(s) < chunk_size_) {
                    good_splits.push_back(s);
                } else {
                    if(!good_splits.empty()) {
                        MergeSplits_(good_splits, merging_separator, final_chunks);
                        good_splits.clear();
                    }
                    if(separators.empty()) {
                        final_chunks.push_back(s);
                    } else {
                        SplitText_(s, separators, final_chunks);
                        // final_chunks.insert(final_chunks.end(), other.begin(), other.end());
                    }
                }
            }

            if(!good_splits.empty()) {
                MergeSplits_(good_splits, merging_separator, final_chunks);
            }
        }
    };

    static TextSplitterPtr CreateRecursiveCharacterTextSplitter(const RecursiveCharacterTextSplitterOptions& options = {}) {
        return std::make_shared<RecursiveCharacterTextSplitter>(options);
    }
}

#endif //RECURSIVECHARACTERTEXTSPLITTER_HPP
