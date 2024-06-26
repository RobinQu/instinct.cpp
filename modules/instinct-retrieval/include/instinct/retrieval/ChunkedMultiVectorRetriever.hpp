//
// Created by RobinQu on 2024/3/12.
//

#ifndef CHUNKEDMULTIVECTORRETRIEVER_HPP
#define CHUNKEDMULTIVECTORRETRIEVER_HPP

#include "document/RecursiveCharacterTextSplitter.hpp"
#include "tokenizer/TiktokenTokenizer.hpp"
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
                     auto parts = parent_splitter_->SplitText(UnicodeString::fromUTF8(doc.text())) | std::views::transform([&,doc](const UnicodeString &str) {
                                 Document document;
                                 str.toUTF8String(*document.mutable_text());
                                 document.mutable_metadata()->CopyFrom(doc.metadata());
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
                document.mutable_metadata()->CopyFrom(parent_doc.metadata());
                const auto file_source = DocumentUtils::GetStringValueMetadataField(parent_doc, METADATA_SCHEMA_FILE_SOURCE_KEY);
                assert_true(file_source && StringUtils::IsNotBlankString(file_source.value()), "should have found parent_doc_id in parent doc's metadata");
                const auto start_idx = parent_doc.text().find(document.text());
                const auto end_idx = start_idx + document.text().size();
                DocumentUtils::AddPresetMetadataFields(
                    document,
                    parent_doc.id(),
                    ++i,
                    file_source.value(),
                    static_cast<int32_t>(start_idx),
                    static_cast<int32_t>(end_idx)
                );
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



    struct RetrieverOptions {
        int parent_chunk_size = 1000;
        int child_chunk_size = 200;
    };

    static StatefulRetrieverPtr CreateChunkedMultiVectorRetriever(const RetrieverOptions& retriever_options,
    const DocStorePtr& doc_store,
    const VectorStorePtr& vector_store) {
        // default to use tiktoken tokenizer
        const auto tokenizer = TiktokenTokenizer::MakeGPT4Tokenizer();
        const auto child_spliter = CreateRecursiveCharacterTextSplitter(tokenizer, {.chunk_size = retriever_options.child_chunk_size});
        if (retriever_options.parent_chunk_size > 0) {
            assert_true(retriever_options.parent_chunk_size > retriever_options.child_chunk_size, "parent_chunk_size should be larger than child_chunk_size");
            const auto parent_splitter = CreateRecursiveCharacterTextSplitter({.chunk_size = retriever_options.parent_chunk_size});
            return CreateChunkedMultiVectorRetriever(
                doc_store,
                vector_store,
                child_spliter,
                parent_splitter
            );
        }
        return CreateChunkedMultiVectorRetriever(
            doc_store,
            vector_store,
            child_spliter
        );
    }
}

#endif //CHUNKEDMULTIVECTORRETRIEVER_HPP
