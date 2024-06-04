//
// Created by RobinQu on 2024/3/5.
//

#ifndef BASETEXTSPLITTER_HPP
#define BASETEXTSPLITTER_HPP

#include <retrieval.pb.h>

#include <utility>
#include "tools/DocumentUtils.hpp"
#include "TextSplitter.hpp"


namespace  INSTINCT_LLM_NS {
    using namespace U_ICU_NAMESPACE;
    using namespace INSTINCT_CORE_NS;

    class ILenghtCalculator {
    public:
        ILenghtCalculator()=default;
        virtual ~ILenghtCalculator()=default;
        ILenghtCalculator(ILenghtCalculator&&)=delete;
        ILenghtCalculator(const ILenghtCalculator&)=delete;
        virtual size_t GetLength(const UnicodeString& s) =0;
    };

    using LenghtCalculatorPtr = std::shared_ptr<ILenghtCalculator>;

    /**
     * Calcluate length by counting its Unicode code points
     */
    class StringLengthCalculator final: public ILenghtCalculator {
    public:
        size_t GetLength(const UnicodeString &s) override {
            return s.countChar32();
        }
    };

    /**
     * Calculate length using a tokenizer
     */
    class TokenizerBasedLengthCalculator final: public ILenghtCalculator {
        TokenizerPtr tokenizer_;

    public:
        explicit TokenizerBasedLengthCalculator(TokenizerPtr tokenizer)
            : tokenizer_(std::move(tokenizer)) {
        }

        size_t GetLength(const UnicodeString &s) override {
            return tokenizer_->Encode(s, {.allow_special = kAll}).size();
        }
    };

    class BaseTextSplitter : public TextSplitter {
    protected:
        int chunk_size_;
        int chunk_overlap_;
        bool keep_separator_;
        bool strip_whitespace_;
        LenghtCalculatorPtr length_calculator_;
    public:

        BaseTextSplitter(const int chunk_size, const int chunk_overlap, const bool keep_separator,
                         const bool strip_whitespace, LenghtCalculatorPtr length_calculator)
            : chunk_size_(chunk_size),
              chunk_overlap_(chunk_overlap),
              keep_separator_(keep_separator),
              strip_whitespace_(strip_whitespace),
              length_calculator_(std::move(length_calculator)) {
        }

        AsyncIterator<Document> SplitDocuments(const AsyncIterator<Document>& docs_itr) override {
            return rpp::source::create<Document>([&,docs_itr](const auto& observer) {
                docs_itr.subscribe([&](const Document& doc) {
                    const auto chunks = SplitText(UnicodeString::fromUTF8(doc.text()));
                    for (int i = 0; i < chunks.size(); i++) {
                        auto& chunk = chunks[i];
                        Document document;
                        chunk.toUTF8String(*document.mutable_text());
                        // auto* chunk_id_field = document.add_metadata();
                        // chunk_id_field->set_name(CHUNK_DOC_PART_INDEX_KEY);
                        // chunk_id_field->set_int_value(i);
                        // DocumentUtils::AddPresetMetadataFields(document, doc.id(), i+1);
                        document.mutable_metadata()->CopyFrom(doc.metadata());
                        observer.on_next(document);
                    }
                }, [&](const std::exception_ptr& e) {
                    observer.on_error(e);
                }, [&]() {
                    observer.on_completed();
                });
            });
        }

    protected:
        /**
         * Merge partial splits into new strings which have size less than `chunk_size`.
         * @param splits
         * @param separator
         * @param docs
         */
        void MergeSplits_(const std::vector<UnicodeString>& splits, const UnicodeString& separator,
                          std::vector<UnicodeString>& docs) const {
            const auto s_len = length_calculator_->GetLength(separator);
            std::vector<UnicodeString> current_doc;
            size_t total = 0;
            for (const auto& s: splits) {
                const auto d_len = length_calculator_->GetLength(s);
                // if chunk_size is reached, merge partials in `current_doc` a new string and append to `docs`.
                if (total + d_len + (current_doc.empty() ? 0 : s_len) > chunk_size_) {
                    if (!current_doc.empty()) {
                        // std::cout << fmt::format("total={}, chunk_size={}, total + d_len + (current_doc.empty() ? 0 : s_len) = {}", total, chunk_size_, total + d_len + (current_doc.empty() ? 0 : s_len)) << std::endl;
                        // details::print_splits("current_doc: ", current_doc);

                        if (const auto doc = JoinDocs_(current_doc, separator); doc.length()) {
                            docs.push_back(doc);
                        }

                        while (total > chunk_overlap_ || ((total + d_len + (current_doc.empty() ? s_len : 0) > chunk_size_) && total > 0)) {
                            total -= length_calculator_->GetLength(current_doc.front()) + (current_doc.size() > 1 ? s_len : 0);
                            current_doc.erase(current_doc.begin());
                        }

                        // if (chunk_overlap_ != 0) {
                        //     // handle overlapping
                        //     while (total > chunk_overlap_ && !current_doc.empty()) {
                        //         // strip first item until remianing chunks are enough for overlapping
                        //         total -= length_calculator_->GetLength(current_doc.front()) + (current_doc.empty() ? 0 : s_len);
                        //         current_doc.erase(current_doc.begin());
                        //     }
                        // } else {
                        //     total = 0;
                        //     current_doc.clear();
                        // }
                    }
                }
                current_doc.push_back(s);
                total += d_len + (current_doc.size() > 1 ? s_len: 0);
            }
            if (const auto rest = JoinDocs_(current_doc, separator); rest.length()) {
                docs.push_back(rest);
            }
        }

        [[nodiscard]] UnicodeString JoinDocs_(const std::vector<UnicodeString>& docs,
                                              const UnicodeString& separator) const {
            UnicodeString text;
            for (int i = 0; i < docs.size(); i++) {
                text += docs[i];
                if (i != docs.size()-1) {
                    text += separator;
                }
            }
            if (strip_whitespace_) {
                return text.trim();
            }
            return text;
        }
    };
}

#endif //BASETEXTSPLITTER_HPP
