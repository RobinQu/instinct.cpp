//
// Created by RobinQu on 2024/6/4.
//
#include <gtest/gtest.h>

#include <instinct/assistant_test_global.hpp>
#include <instinct/assistant/v2/toolkit/summary_guided_file_search.hpp>
#include <instinct/retrieval/vector_store_retriever.hpp>


namespace INSTINCT_ASSISTANT_NS::v2 {
    class TestSummaryGuidedFileSearch: public BaseAssistantApiTest {
    public:
        DuckDBConnectionPoolPtr connection_pool_ = CreateDuckDBConnectionPool(duck_db_);
        EmbeddingsPtr embedding_model_ = CreateOpenAIEmbeddingModel();
        VectorStoreOperatorPtr vector_store_operator_ = CreateDuckDBStoreOperator(duck_db_, embedding_model_, vector_store_metadata_data_mapper_);
        RetrieverOperatorPtr retriever_operator_ = CreateSimpleRetrieverOperator(vector_store_operator_, duck_db_, {.table_name = "docs_" + ChronoUtils::GetCurrentTimestampString()});
        VectorStoreServicePtr vector_store_service_ = CreateVectorStoreService();
        std::filesystem::path asset_dir = std::filesystem::current_path() / "_assets";
    };

    TEST_F(TestSummaryGuidedFileSearch, SimpleSearch) {
        // there files are academic papers that issued at 2024, so the model is unlikely trained with related data
        const auto ingestor1 = RetrieverObjectFactory::CreateIngestor(
                {
                    .file_path = asset_dir / "2405.21048v1.pdf",
                    .file_source_id = "fake-file-1",
                    .fail_fast = true
                }
            );
        const auto ingestor2 = RetrieverObjectFactory::CreateIngestor({
                .file_path = asset_dir / "2405.21060v1.pdf",
                .file_source_id = "fake-file-2",
                .fail_fast = true
            });

        // build retriever
        const auto vdb = vector_store_operator_->CreateInstance("test-vs");
        const auto retriever = CreateVectorStoreRetriever(vdb);
        const auto splitter = CreateRecursiveCharacterTextSplitter({.chunk_size = 800, .chunk_overlap = 400});
        retriever->Ingest(ingestor1->LoadWithSplitter(splitter));
        retriever->Ingest(ingestor2->LoadWithSplitter(splitter));

        // fake two file objects
        VectorStoreFileObject vector_store_file_object1;
        vector_store_file_object1.set_summary("Kaleido Diffusion: Improving Conditional Diffusion Models with Autoregressive Latent Modeling.");
        vector_store_file_object1.set_file_id("fake-file-1");

        VectorStoreFileObject vector_store_file_object2;
        vector_store_file_object2.set_summary("Transformers are SSMs: Generalized Models and Efficient Algorithms Through Structured State Space Duality");
        vector_store_file_object2.set_file_id("fake-file-2");

        const auto tool = CreateSummaryGuidedFileSearch(
            CreateLocalRankingModel(ModelType::BGE_M3_RERANKER),
            retriever,
            {vector_store_file_object1, vector_store_file_object2},
            {.top_file_n = 1}
            );
        LOG_INFO("schema {}", tool->GetSchema().ShortDebugString());
        ASSERT_EQ(tool->GetSchema().name(), "FileSearch");
        ASSERT_TRUE(StringUtils::IsNotBlankString(tool->GetSchema().description()));

        ToolCallObject tool_call_object;
        tool_call_object.set_type(ToolCallObjectType::function);
        tool_call_object.mutable_function()->set_name("FileSearch");
        tool_call_object.mutable_function()->set_arguments(R"({"query": "Tell me what's new in Kaleido Diffusion"})");
        auto result = tool->Invoke(tool_call_object);
        LOG_INFO("result: {}", result.return_value());
        const auto tool_response = ProtobufUtils::Deserialize<SearchToolResponse>(result.return_value());
        ASSERT_TRUE(tool_response.entries_size()>0);
    }
}
