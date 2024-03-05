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

namespace INSTINCT_CORE_NS {

    // using namespace U_ICU_NAMESPACE;

    static std::vector<UnicodeString> DEFAULT_SEPERATOR_FOR_TEXT_SPLITTER = {
        "\n\n",
        "\n",
        " ",
        ""
    };


    struct RecursiveCharacterTextSplitterOptions {
        LengthFunction length_function = IdentityLengthFunction;
        int chunk_size=4000;
        bool keep_sepeartor=true;
        bool strip_whitespace=true;
        std::vector<UnicodeString> seperators = DEFAULT_SEPERATOR_FOR_TEXT_SPLITTER;
    };


    /**
     * \brief Recusively split text using a sequnce of given characters as splitter. Copied a lot from Langchain Python.
     */
    class RecursiveCharacterTextSplitter: public BaseTextSplitter {
        std::vector<UnicodeString> seperators_;
    public:
        explicit RecursiveCharacterTextSplitter(const RecursiveCharacterTextSplitterOptions& options = {}): BaseTextSplitter(
            options.chunk_size,
            0,
            options.keep_sepeartor,
            options.strip_whitespace,
            options.length_function),
        seperators_(options.seperators) {

        }

        static RecursiveCharacterTextSplitter* FromTiktokenTokenizer(TiktokenTokenizer* tokenizer, RecursiveCharacterTextSplitterOptions options = {}) {
            options.length_function = [=](const UnicodeString& str) {
                return tokenizer->Encode(str).size();
            };
            return new RecursiveCharacterTextSplitter(options);
        }

        std::vector<UnicodeString> SplitText(const UnicodeString& text) override {
            auto seps = std::vector(seperators_);
            std::vector<UnicodeString> result;
            SplitText_(text, seps, result);
            return result;
        }

    private:
        void SplitText_(const UnicodeString& text, std::vector<UnicodeString>& seperators, std::vector<UnicodeString>& final_chunks) { // NOLINT(*-no-recursion)
            // default to last sep, assuming it's most common case in text
            UnicodeString seperator = details::escape_for_regular_expression(seperators.back());
            for(auto itr=seperators.begin(); itr!=seperators.end(); ++itr) {
                auto sep = *itr;
                // break if it's empty string
                if(sep == "") {
                    seperator = details::escape_for_regular_expression(sep);
                    seperators.clear();
                    break;
                }
                // break if text can be spllited by sep
                if(text.indexOf(sep) > 0) {
                    seperator = details::escape_for_regular_expression(sep);
                    // seperators = std::vector(itr+1, seperators.end());
                    seperators.erase(seperators.begin(), itr+1);
                    break;
                }
            }

            const auto splits = details::split_text_with_seperator(text, seperator, keep_sepeartor_);
            std::vector<UnicodeString> good_splits;
            const auto merging_seperator = keep_sepeartor_ ? "" : seperator;
            for(auto& s: splits) {
                if(length_function_(s) < chunk_size_) {
                    good_splits.push_back(s);
                } else {
                    if(!good_splits.empty()) {
                        MergeSplits_(good_splits, merging_seperator, final_chunks);
                        good_splits.clear();
                    }
                    if(seperators.empty()) {
                        final_chunks.push_back(s);
                    } else {
                        SplitText_(s, seperators, final_chunks);
                        // final_chunks.insert(final_chunks.end(), other.begin(), other.end());
                    }
                }
            }

            if(!good_splits.empty()) {
                MergeSplits_(good_splits, merging_seperator, final_chunks);
            }
        }




    };
}

#endif //RECURSIVECHARACTERTEXTSPLITTER_HPP
