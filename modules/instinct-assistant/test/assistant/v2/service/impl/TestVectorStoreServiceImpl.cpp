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

}