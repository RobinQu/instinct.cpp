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
    class ChunkedMultiVectorRetriever final : public MultiVectorRetriever {

        TextSplitterPtr child_splitter_;
        TextSplitterPtr parent_splitter_;

    public:
        ChunkedMultiVectorRetriever(
                DocStorePtr doc_store,
                VectorStorePtr vector_store,
                TextSplitterPtr child_splitter,
                TextSplitterPtr parent_splitter = nullptr,
                MultiVectorRetrieverOptions options = {}
        )
                : MultiVectorRetriever(
                std::move(doc_store),
                std::move(vector_store),
                [this](const Document &doc) { return SplitChildDoc_(doc); },
                options),
                  child_splitter_(std::move(child_splitter)),
                  parent_splitter_(std::move(parent_splitter)) {
            assert_true(child_splitter_, "should provide child_splitter");
        }


        void Ingest(const AsyncIterator<Document> &input) override {
            if (parent_splitter_) {
                auto chunked_input = input
                 | rpp::operators::flat_map([&](const Document &doc) {
                     int i = 0;
                    auto parts = parent_splitter_->SplitText(UnicodeString::fromUTF8(doc.text())) |
                         std::views::transform([&](const UnicodeString &str) {
                             Document document;
                             str.toUTF8String(*document.mutable_text());
                             DocumentUtils::AddPresetMetadataFields(document, doc.id(), ++i);
                             return document;
                         });
                    return rpp::source::from_iterable(std::vector<Document> {parts.begin(), parts.end()});
                });
                MultiVectorRetriever::Ingest(chunked_input);
                return;
            }
            MultiVectorRetriever::Ingest(input);
        }

    private:
        [[nodiscard]] std::vector<Document>
        SplitChildDoc_(const Document &parent_doc) const { // NOLINT(*-convert-member-functions-to-static)
            std::vector<Document> results;
            for(int i=0; const auto& str: child_splitter_->SplitText(UnicodeString::fromUTF8(parent_doc.text()))) {
                Document document;
                str.toUTF8String(*document.mutable_text());
                const auto parent_doc_id = DocumentUtils::GetStringValueMetadataField(parent_doc, METADATA_SCHEMA_PARENT_DOC_ID_KEY);
                assert_true(parent_doc_id && StringUtils::IsNotBlankString(parent_doc_id.value()), "should have found parent_doc_id in parent doc's metadata");
                DocumentUtils::AddPresetMetadataFields(document, parent_doc_id.value(), ++i, parent_doc.id());
                LOG_DEBUG("chunked doc: size={}, parent_id={}", document.text().size(), parent_doc.id());
                results.push_back(document);
            }
            return results;
        }
    };

    static StatefulRetrieverPtr CreateChunkedMultiVectorRetriever(
            const DocStorePtr& doc_store,
            const VectorStorePtr& vector_store,
            const TextSplitterPtr& child_splitter,
            const TextSplitterPtr& parent_splitter = nullptr,
            const MultiVectorRetrieverOptions& options = {}
    ) {
        return std::make_shared<ChunkedMultiVectorRetriever>(
                doc_store,
                vector_store,
                child_splitter,
                parent_splitter,
                options
        );
    }
}

#endif //CHUNKEDMULTIVECTORRETRIEVER_HPP
