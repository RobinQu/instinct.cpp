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
        bool keep_sepeartor_;
        bool strip_whitespace_;
        LenghtCalculatorPtr lenght_calculator_;

    public:
        BaseTextSplitter();

        BaseTextSplitter(const int chunk_size, const int chunk_overlap, const bool keep_sepeartor,
                         const bool strip_whitespace, LenghtCalculatorPtr lenght_calculator)
            : chunk_size_(chunk_size),
              chunk_overlap_(chunk_overlap),
              keep_sepeartor_(keep_sepeartor),
              strip_whitespace_(strip_whitespace),
              lenght_calculator_(std::move(lenght_calculator)) {
        }

        AsyncIterator<Document> SplitDocuments(const AsyncIterator<Document>& docs_itr) override {
            return rpp::source::create<Document>([&,docs_itr](const auto& observer) {
                docs_itr.subscribe([&](const auto& doc) {
                    auto chunks = SplitText(UnicodeString::fromUTF8(doc.text()));
                    for (int i = 0; i < chunks.size(); i++) {
                        auto& chunk = chunks[i];
                        Document document;
                        chunk.toUTF8String(*document.mutable_text());
                        // auto* chunk_id_field = document.add_metadata();
                        // chunk_id_field->set_name(CHUNK_DOC_PART_INDEX_KEY);
                        // chunk_id_field->set_int_value(i);
                        DocumentUtils::AddPresetMetadataFileds(document, doc.id(), i+1);
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
        void MergeSplits_(const std::vector<UnicodeString>& splits, const UnicodeString& seperator,
                          std::vector<UnicodeString>& docs) const {
            const auto s_len = lenght_calculator_->GetLength(seperator);
            // std::vector<UnicodeString> docs;
            std::vector<UnicodeString> current_doc;
            size_t total = 0;
            for (const auto& s: splits) {
                const auto d_len = lenght_calculator_->GetLength(s);
                if (total + d_len + (current_doc.empty() ? 0 : s_len) > chunk_size_) {
                    if (!current_doc.empty()) {
                        if (const auto doc = JoinDocs_(current_doc, seperator); doc.length()) {
                            docs.push_back(doc);
                        }
                        if (chunk_overlap_ != 0) {
                            // handle overlapping
                            while (total > chunk_overlap_ && !current_doc.empty()) {
                                // strip first item until remianing chunks are enough for overlapping
                                total -= lenght_calculator_->GetLength(current_doc.front()) + (current_doc.empty() ? 0 : s_len);
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

        [[nodiscard]] UnicodeString JoinDocs_(const std::vector<UnicodeString>& docs,
                                              const UnicodeString& seperator) const {
            UnicodeString text;
            for (int i = 0; i < docs.size(); i++) {
                text += docs[i];
                if (i + 1 < docs.size()) {
                    text += seperator;
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
