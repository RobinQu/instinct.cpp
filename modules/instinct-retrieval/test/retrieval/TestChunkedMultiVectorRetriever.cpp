//
// Created by RobinQu on 2024/3/13.
//
#include <gtest/gtest.h>

#include "retrieval/ChunkedMultiVectorRetriever.hpp"
#include "RetrievalTestGlobals.hpp"
#include "RetrieverObjectFactory.hpp"
#include "store/duckdb/DuckDBDocStore.hpp"
#include "store/duckdb/DuckDBVectorStore.hpp"
#include "document/RecursiveCharacterTextSplitter.hpp"
#include "ingestor/DirectoryTreeIngestor.hpp"
#include "ingestor/ParquetFileIngestor.hpp"


namespace INSTINCT_RETRIEVAL_NS {

    class ChunkedMultiVectorRetrieverTest : public ::testing::Test {

    protected:
        void SetUp() override {
            SetupLogging();
            auto root_path = INSTINCT_LLM_NS::ensure_random_temp_folder();
            std::cout << "MultiVectorRetrieverTest at " << root_path << std::endl;

            auto schema_builder = MetadataSchemaBuilder::Create();
            schema_builder->DefineString("parent_doc_id");
            auto meta_schema = schema_builder->Build();

            size_t dimension = 4096;
            EmbeddingsPtr embedding_model = INSTINCT_LLM_NS::create_pesudo_embedding_model(dimension);
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
            recipes_ingestor_ = RetrieverObjectFactory::CreateDirectoryTreeIngestor(recipes_dir);
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

    /**
     * This should be inside `instinct-core`. But some classes are absent there.
     */
    TEST_F(ChunkedMultiVectorRetrieverTest, SplitterConsistentcyTest) {
        std::ifstream splits_json_file(asset_dir_ / "huggingface_doc_splits.json");

        auto tokenizer = TiktokenTokenizer::MakeGPT4Tokenizer();

        for (const auto splits_collection = nlohmann::json::parse(splits_json_file); const auto& dataset: splits_collection) {
            const auto chunk_size = dataset.at("chunk_size").get<int>();
            const auto chunk_overlap = dataset.at("chunk_overlap").get<int>();
            const auto limit = dataset.at("limit").get<int>();
            LOG_INFO("Settings: chunk_size={}, chunk_overlap={}, limit={}", chunk_size, chunk_overlap, limit);
            auto splitter = CreateRecursiveCharacterTextSplitter(tokenizer, {
                .chunk_size = chunk_size,
                .chunk_overlap=chunk_overlap,
                .keep_separator = true,
                .strip_whitespace = true,
                .separators = {"\n\n", "\n", ".", " ", ""}
            });
            const auto ingest = CreateParquetIngestor(asset_dir_ / "hunggface_doc_train.parquet", "0:text,1:metadata:file_source:varchar", {.limit = static_cast<size_t>(limit)});
            const auto doc_itr = splitter->SplitDocuments(ingest->Load());
            const auto chunked_docs = CollectVector(doc_itr);

            for(int i=0;i<chunked_docs.size();++i) {
                LOG_INFO("Asserting No.{} of total {} docs", i, chunked_docs.size());
                const auto expected_text = dataset.at("texts")[i].get<std::string>();
                ASSERT_EQ(chunked_docs[i].text(), expected_text);
            }
        }

    }

}

