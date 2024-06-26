//
// Created by RobinQu on 2024/6/4.
//
#include <gtest/gtest.h>
#include <fstream>
#include <instinct/AssistantTestGlobals.hpp>
#include <instinct/assistant/v2/task_handler/FileObjectTaskHandler.hpp>

namespace INSTINCT_ASSISTANT_NS::v2 {
    class TestFileObjectTaskHandler: public BaseAssistantApiTest {
    public:
        EmbeddingsPtr embedding_model_ = CreateOpenAIEmbeddingModel();
        VectorStoreOperatorPtr vector_store_operator_ = CreateDuckDBStoreOperator(duck_db_, embedding_model_, vector_store_metadata_data_mapper_);
        RetrieverOperatorPtr retriever_operator_ = CreateSimpleRetrieverOperator(vector_store_operator_, duck_db_, {.table_name = "docs_" + ChronoUtils::GetCurrentTimestampString()});
        VectorStoreServicePtr vector_store_service_ = CreateVectorStoreService(nullptr, retriever_operator_);
        FileServicePtr file_service_ = CreateFileService();
        std::filesystem::path asset_dir_ = std::filesystem::current_path() / "_assets";
        ChatModelPtr chat_model_ = CreateOpenAIChatModel();
        SummaryChainPtr summary_chain = CreateSummaryChain(chat_model_);
    };

    TEST_F(TestFileObjectTaskHandler, SimpleFileHandling) {
        FileObjectTaskHandler task_handler {retriever_operator_, vector_store_service_, file_service_, summary_chain};

        // upload pdf file
        UploadFileRequest upload_file_request;
        upload_file_request.set_filename("Double sovereign.txt");
        upload_file_request.set_purpose(FileObjectPurpose::assistants);
        std::fstream fstream  {asset_dir_ / "Double sovereign.txt", std::ios::binary | std::ios::in};
        const auto file_object1 = file_service_->UploadFile(upload_file_request, fstream);
        fstream.close();
        ASSERT_TRUE(file_object1);

        // create vs
        CreateVectorStoreRequest create_vector_store_request;
        create_vector_store_request.set_name("test-vs");
        const auto vector_store_object1 = vector_store_service_->CreateVectorStore(create_vector_store_request);
        ASSERT_TRUE(vector_store_object1);
        LOG_INFO("vector_store_object1={}", vector_store_object1->ShortDebugString());

        // create vs_file
        CreateVectorStoreFileRequest create_vector_store_file_request;
        create_vector_store_file_request.set_file_id(file_object1->id());
        create_vector_store_file_request.set_vector_store_id(vector_store_object1->id());
        const auto vector_store_file_object1 = vector_store_service_->CreateVectorStoreFile(create_vector_store_file_request);
        ASSERT_TRUE(vector_store_file_object1);
        LOG_INFO("vector_store_file_object1={}", vector_store_file_object1->ShortDebugString());

        CommonTaskScheduler::Task task {
            .task_id = vector_store_file_object1->file_id(),
            .category =  FileObjectTaskHandler::CATEGORY,
            .payload = ProtobufUtils::Serialize(vector_store_file_object1.value())
        };

        // test accept
        ASSERT_TRUE(task_handler.Accept(task));

        // test handle
        ASSERT_NO_THROW({
            task_handler.Handle(task);
        });

        // validate vs_file
        GetVectorStoreFileRequest get_vector_store_file_request;
        get_vector_store_file_request.set_vector_store_id(vector_store_object1->id());
        get_vector_store_file_request.set_file_id(vector_store_file_object1->file_id());
        const auto vector_store_file2 = vector_store_service_->GetVectorStoreFile(get_vector_store_file_request);
        ASSERT_TRUE(vector_store_file2);
        LOG_INFO("vector_store_file2={}", vector_store_file2->ShortDebugString());
        ASSERT_EQ(vector_store_file2->status(), completed);
        ASSERT_TRUE(StringUtils::IsNotBlankString(vector_store_file2->summary()));

        // validate retriever
        const auto retriever = retriever_operator_->GetStatefulRetriever(vector_store_object1->id());
        ASSERT_TRUE(retriever);
        const auto count1 = retriever->GetDocStore()->CountDocuments();
        ASSERT_GT(count1, 0);

        // validate docs
        FindRequest find_request;
        find_request.mutable_query()->mutable_term()->set_name(METADATA_SCHEMA_PARENT_DOC_ID_KEY);
        find_request.mutable_query()->mutable_term()->mutable_term()->set_string_value(vector_store_file_object1->file_id());
        const auto all_doc_itr = retriever->GetDocStore()->FindDocuments(find_request);
        const auto all_docs = CollectVector(all_doc_itr);
        // all inserted docs should have the same parent_doc_id
        ASSERT_EQ(all_docs.size(), count1);
    }

}