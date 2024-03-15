//
// Created by RobinQu on 2024/3/12.
//

#ifndef CHUNKEDMULTIVECTORRETRIEVER_HPP
#define CHUNKEDMULTIVECTORRETRIEVER_HPP

#include <document/TextSplitter.hpp>

#include "MultiVectorRetriever.hpp"
#include "RetrievalGlobals.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    /**
     * Parent-child retriever patterns
     */
    class ChunkedMultiVectorRetriever final: public MultiVectorRetriever {

        TextSplitterPtr child_splitter_;
        TextSplitterPtr parent_splitter_;

    public:
        ChunkedMultiVectorRetriever(DocStorePtr doc_store, VectorStorePtr vector_store,
            MultiVectorRetrieverOptions options, TextSplitterPtr child_splitter,
            TextSplitterPtr parent_splitter)
            : MultiVectorRetriever(
                  std::move(doc_store),
                  std::move(vector_store),
                  [this](const Document& doc) { return SplitChildDoc_(doc); },
                  std::move(options)),
              child_splitter_(std::move(child_splitter)),
              parent_splitter_(std::move(parent_splitter)) {
        }


        void Ingest(const AsyncIterator<Document>& input) override {
            if (parent_splitter_) {
                auto chunked_input = input | rpp::operators::flat_map([&](const Document& doc) {
                    auto parts = parent_splitter_->SplitText(UnicodeString::fromUTF8(doc.text())) | std::views::transform([](const UnicodeString& str) {
                        Document document;
                        str.toUTF8String(*document.mutable_text());
                        return document;
                    });
                    return rpp::source::from_iterable(parts).as_dynamic();
                });
                MultiVectorRetriever::Ingest(chunked_input);
                return;
            }
            MultiVectorRetriever::Ingest(input);
        }

    private:
        [[nodiscard]] std::vector<Document> SplitChildDoc_(const Document& parent_doc) const { // NOLINT(*-convert-member-functions-to-static)
            auto parts = child_splitter_->SplitText(UnicodeString::fromUTF8(parent_doc.text())) | std::views::transform([](const UnicodeString& str) {
                Document document;
                str.toUTF8String(*document.mutable_text());
                return document;
            });
            return {parts.begin(), parts.end()};

        }
    };
}

#endif //CHUNKEDMULTIVECTORRETRIEVER_HPP
