//
// Created by RobinQu on 2024/5/31.
//
#include <gtest/gtest.h>
#include "assistant/v2/service/impl/VectorStoreServiceImpl.hpp"
#include "AssistantTestGlobals.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {

    class VectorStoreServiceTest: public BaseAssistantApiTest {

    };

    TEST_F(VectorStoreServiceTest, VectorStoreCRUD) {
        auto diff = util::MessageDifferencer {};
        auto vector_store_service = CreateVectorStoreService();

        // create
        CreateVectorStoreRequest create_vector_store_request;
        create_vector_store_request.set_name("Art of war");
        create_vector_store_request.add_file_ids("file-1");
        create_vector_store_request.add_file_ids("file-2");
        const auto obj1 = vector_store_service->CreateVectorStore(create_vector_store_request);
        LOG_INFO("obj1={}", obj1->ShortDebugString());
        ASSERT_TRUE(obj1);

        // get
        GetVectorStoreRequest get_vector_store_request;
        get_vector_store_request.set_vector_store_id(obj1->id());
        const auto obj2 = vector_store_service->GetVectorStore(get_vector_store_request);
        LOG_INFO("obj2={}", obj2->ShortDebugString());
        ASSERT_TRUE(diff.Compare(obj1.value(), obj2.value()));

        // list
        ListVectorStoresRequest list_vector_stores_request;
        const auto list1 = vector_store_service->ListVectorStores(list_vector_stores_request);
        LOG_INFO("list1={}", list1.ShortDebugString());
        ASSERT_EQ(list1.object(), "list");
        ASSERT_EQ(list1.data_size(), 1);
        ASSERT_FALSE(list1.has_more());
        ASSERT_EQ(list1.first_id(), list1.data(0).id());
        ASSERT_EQ(list1.last_id(), list1.data(0).id());
        ASSERT_TRUE(diff.Compare(list1.data(0), obj1.value()));

        // update
        ModifyVectorStoreRequest modify_vector_store_request;
        modify_vector_store_request.set_status(VectorStoreObject_VectorStoreStatus_completed);
        modify_vector_store_request.set_summary("hello world");
        modify_vector_store_request.set_vector_store_id(obj1->id());
        const auto obj3 = vector_store_service->ModifyVectorStore(modify_vector_store_request);
        ASSERT_TRUE(obj3);
        ASSERT_EQ(obj3->status(), VectorStoreObject_VectorStoreStatus_completed);

        // set files as completed or VS won't be deleted
        for(const auto& file_id: create_vector_store_request.file_ids()) {
            ModifyVectorStoreFileRequest modify_vector_store_file_request;
            modify_vector_store_file_request.set_vector_store_id(obj1->id());
            modify_vector_store_file_request.set_file_id(file_id);
            modify_vector_store_file_request.set_status(completed);
            ASSERT_TRUE(vector_store_service->ModifyVectorStoreFile(modify_vector_store_file_request));
        }
        // remove
        DeleteVectorStoreRequest delete_vector_store_request;
        delete_vector_store_request.set_vector_store_id(obj1->id());
        const auto obj4 = vector_store_service->DeleteVectorStore(delete_vector_store_request);
        ASSERT_TRUE(obj4.deleted());
        ASSERT_EQ(obj4.object(), "vector_store.deleted");
        ASSERT_EQ(obj4.id(), obj1->id());

        const auto obj5 = vector_store_service->GetVectorStore(get_vector_store_request);
        ASSERT_FALSE(obj5);
    }

    TEST_F(VectorStoreServiceTest, VectorStoreFileCRUD) {
        auto vector_store_service = CreateVectorStoreService();
        auto diff = util::MessageDifferencer {};

        // create
        CreateVectorStoreFileRequest create_vector_store_file_request;
        create_vector_store_file_request.set_vector_store_id("vs-1");
        create_vector_store_file_request.set_file_id("file-1");
        const auto obj1 = vector_store_service->CreateVectorStoreFile(create_vector_store_file_request);
        ASSERT_TRUE(obj1);

        // get
        GetVectorStoreFileRequest get_vector_store_file_request;
        get_vector_store_file_request.set_vector_store_id("vs-1");
        get_vector_store_file_request.set_file_id("file-1");
        const auto obj2 = vector_store_service->GetVectorStoreFile(get_vector_store_file_request);
        ASSERT_TRUE(diff.Compare(obj1.value(), obj2.value()));

        // update
        ModifyVectorStoreFileRequest modify_vector_store_file_request;
        modify_vector_store_file_request.set_status(completed);
        modify_vector_store_file_request.set_vector_store_id("vs-1");
        modify_vector_store_file_request.set_file_id("file-1");
        const auto obj3 = vector_store_service->ModifyVectorStoreFile(modify_vector_store_file_request);
        ASSERT_TRUE(obj3);
        ASSERT_FLOAT_EQ(obj3->status(), completed);

        // list
        ListVectorStoreFilesRequest list_vector_store_files_request;
        list_vector_store_files_request.set_vector_store_id("vs-1");
        const auto list1 = vector_store_service->ListVectorStoreFiles(list_vector_store_files_request);
        ASSERT_EQ(list1.data_size(), 1);
        ASSERT_EQ(list1.object(), "list");
        ASSERT_EQ(list1.first_id(), list1.data(0).file_id());
        ASSERT_EQ(list1.last_id(), list1.data(0).file_id());
        ASSERT_FALSE(list1.has_more());
        ASSERT_TRUE(diff.Compare(list1.data(0), obj3.value()));

        // delete
        DeleteVectorStoreFileRequest delete_vector_store_file_request;
        delete_vector_store_file_request.set_vector_store_id("vs-1");
        delete_vector_store_file_request.set_file_id("file-1");
        const auto obj4 = vector_store_service->DeleteVectorStoreFile(delete_vector_store_file_request);
        ASSERT_TRUE(obj4.deleted());
        ASSERT_EQ(obj4.object(), "vector_store.file.deleted");
        ASSERT_EQ(obj4.id(), "file-1");
    }

    TEST_F(VectorStoreServiceTest, VectorStoreFileBatchCRUD) {
        auto vector_store_service = CreateVectorStoreService();
        auto diff = util::MessageDifferencer {};

        // create
        CreateVectorStoreFileBatchRequest create_vector_store_file_batch_request;
        create_vector_store_file_batch_request.set_vector_store_id("vs-1");
        create_vector_store_file_batch_request.add_file_ids("file-1");
        create_vector_store_file_batch_request.add_file_ids("file-2");
        create_vector_store_file_batch_request.add_file_ids("file-3");
        const auto obj1 = vector_store_service->CreateVectorStoreFileBatche(create_vector_store_file_batch_request);
        ASSERT_TRUE(obj1);

        // list files
        ListFilesInVectorStoreBatchRequest list_files_in_vector_store_batch_request;
        list_files_in_vector_store_batch_request.set_vector_store_id(create_vector_store_file_batch_request.vector_store_id());
        list_files_in_vector_store_batch_request.set_batch_id(obj1->id());
        const auto list1 = vector_store_service->ListFilesInVectorStoreBatch(list_files_in_vector_store_batch_request);
        ASSERT_EQ(list1.data_size(), 3);
        ASSERT_EQ(list1.first_id(), list1.data().begin()->file_id());
        ASSERT_EQ(list1.last_id(), list1.data().rbegin()->file_id());
        ASSERT_FALSE(list1.has_more());

        // list pending
        ListPendingFileBatchObjectsRequest file_batch_objects_request;
        file_batch_objects_request.set_limit(10);
        const auto list2 = vector_store_service->ListPendingFileBatcheObjects(file_batch_objects_request);
        ASSERT_EQ(list2.size(), 1);
        ASSERT_TRUE(diff.Compare(list2.at(0), obj1.value()));

        // update
        ModifyVectorStoreFileBatchRequest modify_vector_store_file_batch_request;
        modify_vector_store_file_batch_request.set_batch_id(obj1->id());
        modify_vector_store_file_batch_request.set_vector_store_id(obj1->vector_store_id());
        modify_vector_store_file_batch_request.set_status(VectorStoreFileBatchObject_VectorStoreFileBatchStatus_failed);
        modify_vector_store_file_batch_request.mutable_last_error()->set_code(CommonErrorType::invalid_request_error);
        modify_vector_store_file_batch_request.set_sanity_check_at(ChronoUtils::GetCurrentEpochMicroSeconds());
        const auto obj5 = vector_store_service->ModifyVectorStoreFileBatch(modify_vector_store_file_batch_request);
        ASSERT_TRUE(obj5);
        const auto list3 = vector_store_service->ListPendingFileBatcheObjects(file_batch_objects_request);
        ASSERT_EQ(list3.size(), 0);

        // get
        GetVectorStoreFileBatchRequest get_vector_store_file_batch_request;
        get_vector_store_file_batch_request.set_vector_store_id(create_vector_store_file_batch_request.vector_store_id());
        get_vector_store_file_batch_request.set_batch_id(obj1->id());
        const auto obj2 = vector_store_service->GetVectorStoreFileBatch(get_vector_store_file_batch_request);
        ASSERT_TRUE(obj2);
        ASSERT_TRUE(diff.Compare(obj2.value(), obj5.value()));
        ASSERT_EQ(obj2->status(), VectorStoreFileBatchObject_VectorStoreFileBatchStatus_failed);
        ASSERT_EQ(obj2->last_error().code(), invalid_request_error);

        // cancel
        CancelVectorStoreFileBatchRequest cancel_vector_store_file_batch_request;
        cancel_vector_store_file_batch_request.set_vector_store_id(create_vector_store_file_batch_request.vector_store_id());
        cancel_vector_store_file_batch_request.set_batch_id(obj1->id());
        const auto obj3 = vector_store_service->CancelVectorStoreFileBatch(cancel_vector_store_file_batch_request);
        ASSERT_TRUE(obj3);
        ASSERT_EQ(obj3->status(), VectorStoreFileBatchObject_VectorStoreFileBatchStatus_cancelled);

        // get again
        const auto obj4 = vector_store_service->GetVectorStoreFileBatch(get_vector_store_file_batch_request);
        ASSERT_TRUE(obj4);
        ASSERT_TRUE(diff.Compare(obj4.value(), obj3.value()));
    }

}