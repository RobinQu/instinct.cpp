//
// Created by RobinQu on 2024/2/28.
//

#ifndef RECURSIVECHARACTERTEXTSPLITTER_HPP
#define RECURSIVECHARACTERTEXTSPLITTER_HPP
#include "CoreGlobals.hpp"
#include "TextSplitter.hpp"
#include <unicode/regex.h>
#include <unicode/ustring.h>
#include <unicode/brkiter.h>

namespace INSTINCT_CORE_NS {

    using namespace U_ICU_NAMESPACE;

    using LengthFunction = std::function<int32_t(const UnicodeString&)> ;

    namespace details {
        template<int parts_size = 10>
        void split_text_with_regex(const UnicodeString& text, const UnicodeString& seperator, std::vector<UnicodeString>& result) {
            UErrorCode status = U_ZERO_ERROR;
            RegexMatcher matcher(seperator, 0, status);
            UnicodeString parts[parts_size];
            if(U_FAILURE(status)) {
                std::string sep_utf8;
                throw LangchainException("Failed to compile regex with seperator string: " + seperator.toUTF8String(sep_utf8));
            }
            // it's okay that parts_size is relatively small, since TextSplitter will call split_text_with_regex in a tail-recursive style.
            int32_t splits_size = matcher.split(text, parts, parts_size, status);
            if(U_FAILURE(status)) {
                throw LangchainException("Failed to split text with seperator regex");
            }
            result.insert(result.end(), parts, parts+splits_size);
        }


        template<int parts_size = 10>
        static std::vector<UnicodeString> split_text_with_regex(const UnicodeString& text, const UnicodeString& seperator, bool keep_seperator) {
            std::vector<UnicodeString> result;
            if(seperator.length()) {
                UnicodeString parts[parts_size];
                if (keep_seperator) {
                    UnicodeString grouped_sepeartor = "(";
                    grouped_sepeartor.append(seperator);
                    grouped_sepeartor.append(")");
                    auto splits_temp = split_text_with_regex<parts_size>(text, grouped_sepeartor);
                    result.push_back(splits_temp[0]);
                    for(int i=1;i<splits_temp.size();i+=2) {
                        result.push_back(splits_temp[i] + splits_temp[i+1]);
                    }
                    if (splits_temp.size()%2==0) {
                        result.push_back(splits_temp.back());
                    }
                }  else {
                    split_text_with_regex(text, seperator, result);
                }
            } else { // it's empty seperator, so we have to split into a sequence of chars.
                UErrorCode status = U_ZERO_ERROR;
                auto itr = BreakIterator::createCharacterInstance(Locale::getDefault(), status);
                itr->setText(text);
                if(U_FAILURE(status)) {
                    throw LangchainException("Failed to createCharacterInstance");
                }
                // see example at: https://github.com/unicode-org/icu/blob/main/icu4c/source/samples/break/break.cpp
                // copy each *grapheme* not code point into result
                int32_t start = itr->first();
                for(int32_t end = itr->next(); end!=BreakIterator::DONE; start=end, end=itr->next()) {
                    result.push_back(UnicodeString(text, start, end));
                }
                delete itr;

                // StringCharacterIterator itr(text);
                // while (itr.hasNext()) {
                //     result.push_back(itr.next32PostInc());
                // }
            }
            auto parts_view = result | std::views::filter([](const UnicodeString& v) {
                return v.length() > 0;
            });
            return {parts_view.begin(), parts_view.end()};
        }
    }

    /**
     * \brief Recusively split text using a sequnce of given characters as splitter. Copied a lot from Langchain Python.
     */
    class RecursiveCharacterTextSplitter: public TextSplitter {
    public:
        RecursiveCharacterTextSplitter(const int chunk_size, const int chunk_overlap, const bool keep_sepeartor,
            const bool strip_whitespace, LengthFunction length_function, std::vector<UnicodeString> seperators)
            : chunk_size_(chunk_size),
              chunk_overlap_(chunk_overlap),
              keep_sepeartor_(keep_sepeartor),
              strip_whitespace_(strip_whitespace),
              length_function_(std::move(length_function)),
              seperators_(std::move(seperators)) {
        }

        explicit RecursiveCharacterTextSplitter(LengthFunction length_function)
            : length_function_(std::move(length_function)) {
        }

        RecursiveCharacterTextSplitter(LengthFunction length_function, std::vector<UnicodeString> seperators)
            : length_function_(std::move(length_function)),
              seperators_(std::move(seperators)) {
        }


        std::vector<std::string> SplitText(const std::string& text) override {
            auto seps = std::vector(seperators_);
            auto splits = SplitText_(UnicodeString::fromUTF8(text), seps);
            auto string_view = splits | std::views::transform([](const UnicodeString& split) {
                std::string utf8_string;
                return split.toUTF8String(utf8_string);
            });
            return {string_view.begin(), string_view.end()};
        }
    private:
        std::vector<UnicodeString> SplitText_(const UnicodeString& text, std::vector<UnicodeString>& seperators) {
            static auto empty = UnicodeString::fromUTF8("");
            std::vector<UnicodeString> final_chunks;

            // choose last sep, assuming it's most common case in text
            UnicodeString seperator = seperators.back();
            for(const auto& sep: seperators) {
                // break if it's empty string
                if(sep == empty) {
                    seperator = sep;
                    break;
                }
                // break if text can be spllited by sep
                if(text.indexOf(sep) > 0) {
                    seperator = sep;
                    seperators.pop_back();
                    break;
                }
            }

            auto splits = details::split_text_with_regex(text, seperator, keep_sepeartor_);
            std::vector<UnicodeString> good_splits;
            for(auto& s: splits) {
                if(length_function_(s) < chunk_size_) {
                    good_splits.push_back(s);
                } else {
                    if(!good_splits.empty()) {
                        auto merged_text = MergeSplits_(good_splits, seperator);
                        final_chunks.insert(final_chunks.end(), merged_text.begin(), merged_text.end());
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
                auto merged_text = MergeSplits_(good_splits, seperator);
                final_chunks.insert(final_chunks.end(), merged_text.begin(), merged_text.end());
            }
            return final_chunks;
        }

        [[nodiscard]] std::vector<UnicodeString> MergeSplits_(const std::vector<UnicodeString>& splits, const UnicodeString& seperator) const {
            const auto s_len = length_function_(seperator);
            std::vector<UnicodeString> docs;
            std::vector<UnicodeString> current_doc;
            int32_t total = 0;
            for(const auto& s: splits) {
                const auto d_len = length_function_(s);
                if(const auto len1 = total + d_len + (current_doc.empty() ? 0 : s_len); len1 > chunk_size_) {
                    if(total>chunk_size_) {
                        // TODO warns
                    }
                    if(!current_doc.empty()) {
                        if (const auto doc = JoinDocs_(current_doc, seperator); doc.length()) {
                            docs.push_back(doc);
                        }
                        const auto len2 = total + d_len + (current_doc.empty() ? 0 : s_len);
                        while (total>chunk_overlap_ || len2>chunk_size_) {
                            total -= length_function_(current_doc[0]) + (current_doc.size()>1 ? s_len :0);
                            current_doc.erase(current_doc.begin());
                        }
                    }
                }
                current_doc.push_back(s);
                total += d_len + (current_doc.size()>1 ? s_len :0);
            }
            if(const auto doc = JoinDocs_(current_doc, seperator); doc.length()) {
                docs.push_back(doc);
            }
            return docs;
        }

        [[nodiscard]] UnicodeString JoinDocs_(const std::vector<UnicodeString>& docs, const UnicodeString& seperator) const  {
            UnicodeString text;
            for(int i=0; auto&& doc: docs) {
                text+=doc;
                if(++i < docs.size()) {
                    text+=seperator;
                }
            }
            if(strip_whitespace_) {
                return text.trim();
            }
            return text;
        }

        int chunk_size_ = 4000;
        int chunk_overlap_ = 200;
        bool keep_sepeartor_ = false;
        bool strip_whitespace_ = true;
        LengthFunction length_function_;
        std::vector<UnicodeString> seperators_ = {
            "\n\n",
            "\n",
            " ",
            ""
        };
    };
}

#endif //RECURSIVECHARACTERTEXTSPLITTER_HPP
