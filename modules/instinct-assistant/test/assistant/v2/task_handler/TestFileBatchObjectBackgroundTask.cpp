//
// Created by RobinQu on 2024/6/10.
//
#include <gtest/gtest.h>

#include "AssistantTestGlobals.hpp"
#include "assistant/v2/task_handler/FileBatchObjectBackgroundTask.hpp"


namespace INSTINCT_ASSISTANT_NS::v2 {
    class TestFileBatchObjectBackgroundTask: public BaseAssistantApiTest {
    protected:
        EmbeddingsPtr embedding_model_ = CreateOpenAIEmbeddingModel();
        VectorStoreOperatorPtr vector_store_operator_ = CreateDuckDBStoreOperator(duck_db_, embedding_model_, vector_store_metadata_data_mapper_);
        RetrieverOperatorPtr retriever_operator_ = CreateSimpleRetrieverOperator(vector_store_operator_, duck_db_, {.table_name = "docs_" + ChronoUtils::GetCurrentTimestampString()});
        VectorStoreServicePtr vector_store_service_ = CreateVectorStoreService(nullptr, retriever_operator_);
        std::filesystem::path asset_dir_ = std::filesystem::current_path() / "_assets";
        FileServicePtr file_service_ = CreateFileService();


        std::optional<VectorStoreFileBatchObject> CreateVectorStoreFileBatch(
            std::vector<std::string>& file_ids
        ) const {
            // upload files
            for(const auto& file: {"Antimonumento 5J .txt", "Double sovereign.txt"}) {
                UploadFileRequest upload_file_request;
                upload_file_request.set_filename(file);
                upload_file_request.set_purpose(assistants);
                std::fstream fstream  {asset_dir_ / file, std::ios::binary | std::ios::in};
                const auto file_object1 = file_service_->UploadFile(upload_file_request, fstream);
                fstream.close();
                file_ids.push_back(file_object1->id());
            }


            CreateVectorStoreRequest create_vector_store_request;
            create_vector_store_request.set_name("test");
            const auto vs = vector_store_service_->CreateVectorStore(create_vector_store_request);
            LOG_INFO("vs: {}", vs->ShortDebugString());

            CreateVectorStoreFileBatchRequest create_vector_store_file_batch_request;
            create_vector_store_file_batch_request.set_vector_store_id(vs->id());
            create_vector_store_file_batch_request.mutable_file_ids()->Add(file_ids.begin(), file_ids.end());
            const auto file_batch = vector_store_service_->CreateVectorStoreFileBatch(create_vector_store_file_batch_request);
            LOG_INFO("file_batch: {}", file_batch->ShortDebugString());
            assert_true(file_batch->status() == VectorStoreFileBatchObject_VectorStoreFileBatchStatus_in_progress);
            return file_batch;
        }
    };

    TEST_F(TestFileBatchObjectBackgroundTask, Lifecycle) {
        FileBatchObjectBackgroundTask file_batch_object_background_task {vector_store_service_, retriever_operator_};
        file_batch_object_background_task.Start();
        file_batch_object_background_task.Shutdown();
    }

    TEST_F(TestFileBatchObjectBackgroundTask, HandleCompletedBatch) {
        FileBatchObjectBackgroundTask file_batch_object_background_task {vector_store_service_, retriever_operator_};

        std::vector<std::string> file_ids;
        const auto file_batch = CreateVectorStoreFileBatch(file_ids);
        // mock file handling
        for(const auto& file_id: file_ids) {
            ModifyVectorStoreFileRequest modify_vector_store_file_request;
            modify_vector_store_file_request.set_file_id(file_id);
            modify_vector_store_file_request.set_vector_store_id(file_batch->vector_store_id());
            modify_vector_store_file_request.set_status(completed);
            ASSERT_TRUE(vector_store_service_->ModifyVectorStoreFile(modify_vector_store_file_request));
        }

        ASSERT_NO_THROW({
            file_batch_object_background_task.Handle();
        });

        GetVectorStoreFileBatchRequest get_vector_store_file_batch_request;
        get_vector_store_file_batch_request.set_batch_id(file_batch->id());
        get_vector_store_file_batch_request.set_vector_store_id(file_batch->vector_store_id());
        const auto obj2 = vector_store_service_->GetVectorStoreFileBatch(get_vector_store_file_batch_request);
        ASSERT_EQ(obj2->status(), VectorStoreFileBatchObject_VectorStoreFileBatchStatus_completed);
    }

    TEST_F(TestFileBatchObjectBackgroundTask, HandleFailedBatch) {
        FileBatchObjectBackgroundTask file_batch_object_background_task {vector_store_service_, retriever_operator_};

        std::vector<std::string> file_ids;
        const auto file_batch = CreateVectorStoreFileBatch(file_ids);

        // mock file handling
        for(int i=0;const auto& file_id: file_ids) {
            ModifyVectorStoreFileRequest modify_vector_store_file_request;
            modify_vector_store_file_request.set_file_id(file_id);
            modify_vector_store_file_request.set_vector_store_id(file_batch->vector_store_id());
            if(i++%2==0) {
                modify_vector_store_file_request.set_status(completed);
            } else {
                modify_vector_store_file_request.set_status(failed);
                modify_vector_store_file_request.mutable_last_error()->set_code(invalid_request_error);
            }
            ASSERT_TRUE(vector_store_service_->ModifyVectorStoreFile(modify_vector_store_file_request));
        }

        ASSERT_NO_THROW({
            file_batch_object_background_task.Handle();
        });

        GetVectorStoreFileBatchRequest get_vector_store_file_batch_request;
        get_vector_store_file_batch_request.set_batch_id(file_batch->id());
        get_vector_store_file_batch_request.set_vector_store_id(file_batch->vector_store_id());
        const auto obj2 = vector_store_service_->GetVectorStoreFileBatch(get_vector_store_file_batch_request);
        ASSERT_EQ(obj2->status(), VectorStoreFileBatchObject_VectorStoreFileBatchStatus_failed);
        ASSERT_EQ(obj2->last_error().code(), invalid_request_error);
    }

    TEST_F(TestFileBatchObjectBackgroundTask, HandleCancelledFiles) {
        FileBatchObjectBackgroundTask file_batch_object_background_task {vector_store_service_, retriever_operator_};

        std::vector<std::string> file_ids;
        const auto file_batch = CreateVectorStoreFileBatch(file_ids);

        // mock file handling
        for(int i=0;const auto& file_id: file_ids) {
            ModifyVectorStoreFileRequest modify_vector_store_file_request;
            modify_vector_store_file_request.set_file_id(file_id);
            modify_vector_store_file_request.set_vector_store_id(file_batch->vector_store_id());
            if(i++%2==0) {
                modify_vector_store_file_request.set_status(completed);
            }
            ASSERT_TRUE(vector_store_service_->ModifyVectorStoreFile(modify_vector_store_file_request));
        }

        // cancel batch
        CancelVectorStoreFileBatchRequest cancel_vector_store_file_batch_request;
        cancel_vector_store_file_batch_request.set_vector_store_id(file_batch->vector_store_id());
        cancel_vector_store_file_batch_request.set_batch_id(file_batch->id());
        ASSERT_TRUE(vector_store_service_->CancelVectorStoreFileBatch(cancel_vector_store_file_batch_request));

        // test handle
        ASSERT_NO_THROW({
            file_batch_object_background_task.Handle();
        });

        // get file batch
        GetVectorStoreFileBatchRequest get_vector_store_file_batch_request;
        get_vector_store_file_batch_request.set_batch_id(file_batch->id());
        get_vector_store_file_batch_request.set_vector_store_id(file_batch->vector_store_id());
        const auto obj2 = vector_store_service_->GetVectorStoreFileBatch(get_vector_store_file_batch_request);
        ASSERT_EQ(obj2->status(), VectorStoreFileBatchObject_VectorStoreFileBatchStatus_cancelled);
    }

}