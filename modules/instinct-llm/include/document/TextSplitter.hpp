//
// Created by RobinQu on 2024/2/27.
//

#ifndef TEXTSPLITTER_HPP
#define TEXTSPLITTER_HPP

#include <unicode/brkiter.h>

#include "CoreGlobals.hpp"
#include "functional/ReactiveFunctions.hpp"
#include "tokenizer/Tokenizer.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;
    using namespace U_ICU_NAMESPACE;

    class TextSplitter {
    public:
        TextSplitter()=default;
        virtual ~TextSplitter() =default;
        TextSplitter(TextSplitter&&)=delete;
        TextSplitter(const TextSplitter&)=delete;
        virtual std::vector<UnicodeString> SplitText(const UnicodeString& text) = 0;
        virtual AsyncIterator<Document> SplitDocuments(const AsyncIterator<Document>& docs_itr) = 0;

    };
    using TextSplitterPtr = std::shared_ptr<TextSplitter>;



    /**
     * you cannot see things in details
     */
    namespace details {

        static std::vector<UnicodeString> split_text_with_seperator(const UnicodeString& text, const UnicodeString& seperator, const bool keep_seperator) {
            std::vector<UnicodeString> result;
            if(seperator.length()) {
                if (keep_seperator) {
                    UnicodeString grouped_sepeartor = "(";
                    grouped_sepeartor.append(seperator);
                    grouped_sepeartor.append(")");
                    split_text_with_regex(text, grouped_sepeartor, result);
                }  else {
                    split_text_with_regex(text, seperator, result);
                }
            } else { // it's empty seperator, so we have to split into a sequence of chars.
                UErrorCode status = U_ZERO_ERROR;
                const auto locale = Locale::getDefault();
                // LOG_DEBUG("Build BreakIterator with locale: {}", locale.getLanguage());
                const auto itr = BreakIterator::createCharacterInstance(locale, status);
                if(U_FAILURE(status)) {
                    throw InstinctException("Failed to createCharacterInstance: " + std::string(u_errorName(status)));
                }
                itr->setText(text);
                // see example at: https://github.com/unicode-org/icu/blob/main/icu4c/source/samples/break/break.cpp
                // copy each *grapheme* not code point into result
                int32_t start = itr->first();
                for(int32_t end = itr->next(); end!=BreakIterator::DONE; start=end, end=itr->next()) {
                    result.emplace_back(text, start, end-start);
                }
                delete itr;
            }
            auto parts_view = result | std::views::filter([](const UnicodeString& v) {
                return v.length() > 0;
            });
            return {parts_view.begin(), parts_view.end()};
        }
    }

}

#endif //TEXTSPLITTER_HPP
