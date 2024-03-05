//
// Created by RobinQu on 2024/2/28.
//

#ifndef RECURSIVECHARACTERTEXTSPLITTER_HPP
#define RECURSIVECHARACTERTEXTSPLITTER_HPP
#include "CoreGlobals.hpp"
#include "TextSplitter.hpp"
#include <unicode/regex.h>
#include <unicode/ustring.h>
#include "LanguageSplitters.hpp"
#include "tokenizer/TiktokenTokenizer.hpp"
#include "tokenizer/Tokenizer.hpp"

namespace INSTINCT_CORE_NS {

    using namespace U_ICU_NAMESPACE;

    using LengthFunction = std::function<size_t(const UnicodeString&)> ;

    static LengthFunction IdentityLengthFunction = [](const UnicodeString& s) {
        return s.length();
    };

    static std::vector<UnicodeString> DEFAULT_SEPERATOR_FOR_TEXT_SPLITTER = {
        "\n\n",
        "\n",
        " ",
        ""
    };

    struct RecursiveCharacterTextSplitterOptions {
        LengthFunction length_function = IdentityLengthFunction;
        std::vector<UnicodeString> seperators = DEFAULT_SEPERATOR_FOR_TEXT_SPLITTER;
        int chunk_size=4000;
        int chunk_overlap=200;
        bool keep_sepeartor=true;
        bool strip_whitespace=true;
    };


    /**
     * \brief Recusively split text using a sequnce of given characters as splitter. Copied a lot from Langchain Python.
     */
    class RecursiveCharacterTextSplitter: public TextSplitter {
    public:
        RecursiveCharacterTextSplitter()=delete;
        explicit RecursiveCharacterTextSplitter(const RecursiveCharacterTextSplitterOptions& options)
            : chunk_size_(options.chunk_size),
              chunk_overlap_(options.chunk_overlap),
              keep_sepeartor_(options.keep_sepeartor),
              strip_whitespace_(options.strip_whitespace),
              length_function_(options.length_function),
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
            return SplitText_(text, seps);
        }
    private:

        std::vector<UnicodeString> SplitText_(const UnicodeString& text, std::vector<UnicodeString>& seperators) { // NOLINT(*-no-recursion)
            std::vector<UnicodeString> final_chunks;

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
                if(length_function_(s) <= chunk_size_) {
                    good_splits.push_back(s);
                } else {
                    if(!good_splits.empty()) {
                        MergeSplits_(good_splits, merging_seperator, final_chunks);
                        good_splits.clear();
                    }
                    if(seperators.empty()) {
                        final_chunks.push_back(s);
                    } else {
                        auto other = SplitText_(s, seperators);
                        final_chunks.insert(final_chunks.end(), other.begin(), other.end());
                    }
                }
            }

            if(!good_splits.empty()) {
                MergeSplits_(good_splits, merging_seperator, final_chunks);
            }
            return final_chunks;
        }

        void MergeSplits_(const std::vector<UnicodeString>& splits, const UnicodeString& seperator, std::vector<UnicodeString>& docs) const {
            const auto s_len = length_function_(seperator);
            // std::vector<UnicodeString> docs;
            std::vector<UnicodeString> current_doc;
            size_t total = 0;
            for(const auto& s: splits) {
                const auto d_len = length_function_(s);
                if (total + d_len + (current_doc.empty() ? 0: s_len) > chunk_size_) {
                    if(!current_doc.empty()) {
                        if(const auto doc = JoinDocs_(current_doc, seperator); doc.length()) {
                            docs.push_back(doc);
                        }
                        if (chunk_overlap_!=0) {
                            // handle overlapping
                            while(total > chunk_overlap_) {
                                // strip first item until remianing chunks are enough for overlapping
                                total -= d_len + (current_doc.empty() ? 0: s_len);
                                current_doc.erase(current_doc.begin());
                            }
                        } else {
                            total = 0;
                            current_doc.clear();
                        }
                    }
                }
                total += d_len + (current_doc.empty() ? 0 : s_len);
                current_doc.push_back(s);
            }
            if (const auto rest = JoinDocs_(current_doc, seperator); rest.length()) {
                docs.push_back(rest);
            }
        }

        [[nodiscard]] UnicodeString JoinDocs_(const std::vector<UnicodeString>& docs, const UnicodeString& seperator) const  {
            UnicodeString text;
            for(int i=0; i<docs.size(); i++) {
                text+=docs[i];
                if(i+1 < docs.size()) {
                    text+=seperator;
                }
            }
            if(strip_whitespace_) {
                return text.trim();
            }
            return text;
        }

        int chunk_size_;
        int chunk_overlap_;
        bool keep_sepeartor_;
        bool strip_whitespace_;
        LengthFunction length_function_;
        std::vector<UnicodeString> seperators_{};
    };
}

#endif //RECURSIVECHARACTERTEXTSPLITTER_HPP
