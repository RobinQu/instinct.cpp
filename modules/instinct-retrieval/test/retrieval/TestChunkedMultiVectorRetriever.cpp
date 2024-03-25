//
// Created by RobinQu on 2024/3/13.
//
#include <gtest/gtest.h>

#include "retrieval/ChunkedMultiVectorRetriever.hpp"
#include "RetrievalTestGlobals.hpp"
#include "store/duckdb/DuckDBDocStore.hpp"
#include "store/duckdb/DuckDBVectorStore.hpp"
#include "document/RecursiveCharacterTextSplitter.hpp"
#include "ingestor/DirectoryTreeIngestor.hpp"


namespace INSTINCT_RETRIEVAL_NS {

    class ChunkedMultiVectorRetrieverTest : public ::testing::Test {

    protected:
        void SetUp() override {
            SetupLogging();
            auto root_path = instinct::test::ensure_random_temp_folder();
            std::cout << "MultiVectorRetrieverTest at " << root_path << std::endl;

            auto schema_builder = MetadataSchemaBuilder::Create();
            schema_builder->DefineString("parent_doc_id");
            auto meta_schema = schema_builder->Build();

            size_t dimension = 4096;
            EmbeddingsPtr embedding_model = instinct::test::create_pesudo_embedding_model(dimension);
            DuckDBStoreOptions db_options = {
                    .table_name = "document_table",
                    .db_file_path = root_path / "doc_store.db"
            };
            doc_store_ = CreateDuckDBDocStore(db_options);

            DuckDBStoreOptions vec_db_options = {
                    .table_name = "summaries_table",
                    .db_file_path = root_path / "summary_store.db",
                    .dimension = dimension,
            };
            vector_store_ = CreateDuckDBVectorStore(embedding_model, vec_db_options, meta_schema);

            child_splitter_ = CreateRecursiveCharacterTextSplitter({.chunk_size = 100});
            parent_splitter_ = CreateRecursiveCharacterTextSplitter({.chunk_size = 300});

            // corpus data is copied to build tree in CMakeLists.txt
            asset_dir_ = std::filesystem::current_path() / "_corpus";

            // load all recipes in folder
            const auto recipes_dir = asset_dir_  / "recipes";
            std::cout << "reading recipes from " << recipes_dir << std::endl;
            recipes_ingestor_ = CreateDirectoryTreeIngestor(recipes_dir);
        }


        std::filesystem::path asset_dir_;
        ChatModelPtr llm_;
        DocStorePtr doc_store_;
        VectorStorePtr vector_store_;
        StatefulRetrieverPtr chunked_retriever_;
        TextSplitterPtr child_splitter_;
        TextSplitterPtr parent_splitter_;
        IngestorPtr recipes_ingestor_;
    };

    TEST_F(ChunkedMultiVectorRetrieverTest, SimpleRetrieve) {
        // chunking with child splitter only
        chunked_retriever_ = CreateChunkedMultiVectorRetriever(doc_store_, vector_store_, child_splitter_);
        chunked_retriever_->Ingest(recipes_ingestor_->Load());

        auto result_itr = chunked_retriever_->Retrieve({.text = "A quick dish for lunch"});
        auto result_vec = CollectVector(result_itr);
        for(const auto& doc: result_vec) {
            std::cout << doc.DebugString() << std::endl;
        }
        ASSERT_FALSE(result_vec.empty());
    }


    TEST_F(ChunkedMultiVectorRetrieverTest, BuildWithParentSplitter) {
        chunked_retriever_ = CreateChunkedMultiVectorRetriever(doc_store_, vector_store_, child_splitter_, parent_splitter_);
        chunked_retriever_->Ingest(recipes_ingestor_->Load());

        auto result_itr = chunked_retriever_->Retrieve({.text = "A quick dish for lunch"});
        auto result_vec = CollectVector(result_itr);
        for(const auto& doc: result_vec) {
            std::cout << doc.DebugString() << std::endl;
        }
        ASSERT_FALSE(result_vec.empty());
    }

}

