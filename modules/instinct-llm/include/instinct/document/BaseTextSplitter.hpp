//
// Created by RobinQu on 2024/3/5.
//

#ifndef BASETEXTSPLITTER_HPP
#define BASETEXTSPLITTER_HPP

#include <retrieval.pb.h>

#include <utility>
#include <instinct/tools/DocumentUtils.hpp>
#include <instinct/document/TextSplitter.hpp>


namespace  INSTINCT_LLM_NS {
    using namespace U_ICU_NAMESPACE;
    using namespace INSTINCT_CORE_NS;

    class ILengthCalculator {
    public:
        ILengthCalculator()=default;
        virtual ~ILengthCalculator()=default;
        ILengthCalculator(ILengthCalculator&&)=delete;
        ILengthCalculator(const ILengthCalculator&)=delete;
        virtual size_t GetLength(const UnicodeString& s) =0;
    };

    using LengthCalculatorPtr = std::shared_ptr<ILengthCalculator>;

    /**
     * Calcluate length by counting its Unicode code points
     */
    class StringLengthCalculator final: public ILengthCalculator {
    public:
        size_t GetLength(const UnicodeString &s) override {
            return s.countChar32();
        }
    };

    /**
     * Calculate length using a tokenizer
     */
    class TokenizerBasedLengthCalculator final: public ILengthCalculator {
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
        LengthCalculatorPtr length_calculator_;
    public:

        BaseTextSplitter(const int chunk_size, const int chunk_overlap, const bool keep_separator,
                         const bool strip_whitespace, LengthCalculatorPtr length_calculator)
            : chunk_size_(chunk_size),
              chunk_overlap_(chunk_overlap),
              keep_separator_(keep_separator),
              strip_whitespace_(strip_whitespace),
              length_calculator_(std::move(length_calculator)) {
        }

        AsyncIterator<Document> SplitDocuments(const AsyncIterator<Document>& docs_itr) override {
            return rpp::source::create<Document>([&,docs_itr](const auto& observer) {
                docs_itr.subscribe([&](const Document& doc) {
                    for (const auto & chunk : SplitText(UnicodeString::fromUTF8(doc.text()))) {
                        Document document;
                        chunk.toUTF8String(*document.mutable_text());
                        document.mutable_metadata()->CopyFrom(doc.metadata());
                        auto *start_index_field = document.add_metadata();
                        start_index_field->set_name(METADATA_SCHEMA_CHUNK_START_INDEX_KEY);
                        start_index_field->set_int_value(static_cast<int32_t>(doc.text().find(document.text())));
                        auto *end_index_field = document.add_metadata();
                        end_index_field->set_name(METADATA_SCHEMA_CHUNK_END_INDEX_KEY);
                        end_index_field->set_int_value(static_cast<int32_t>(start_index_field->int_value() + document.text().size()));
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
                    }
                }
                current_doc.push_back(s);
                total += d_len + (current_doc.size() > 1 ? s_len: 0);
            }
            if (const auto rest = JoinDocs_(current_doc, separator); !rest.isEmpty()) {
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
