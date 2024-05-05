//
// Created by RobinQu on 2024/4/25.
//
#include <gtest/gtest.h>
#include <google/protobuf/util/message_differencer.h>

#include "AssistantTestGlobals.hpp"
#include "assistant/v2/service/IFileService.hpp"
#include "assistant/v2/service/impl/FileServiceImpl.hpp"

namespace INSTINCT_ASSISTANT_NS {
    class FileServiceTest: public BaseAssistantApiTest {

    };


    TEST_F(FileServiceTest, SimpleCRUD) {
        const auto file_service = CreateFileService();

        // upload file 1
        UploadFileRequest upload_file_request1;
        upload_file_request1.set_file_content("helloworld");
        upload_file_request1.set_filename("test.txt");
        upload_file_request1.set_purpose(assistants);
        const auto obj1 = file_service->UploadFile(upload_file_request1);
        LOG_INFO("upload returned: {}", obj1->ShortDebugString());
        ASSERT_EQ(obj1->filename(), "test.txt");
        ASSERT_GT(obj1->bytes(), 0);
        ASSERT_GT(obj1->created_at(), 0);
        ASSERT_GT(obj1->modified_at(), 0);
        ASSERT_EQ(obj1->object(), "file");

        // upload file 2
        UploadFileRequest upload_file_request2;
        const auto sql_string = "select * from tbl";
        upload_file_request2.set_file_content(sql_string);
        upload_file_request2.set_filename("up.sql");
        upload_file_request2.set_purpose(assistants_output);
        const auto obj2 = file_service->UploadFile(upload_file_request2);
        LOG_INFO("upload returned: {}", obj2->ShortDebugString());
        ASSERT_EQ(obj2->filename(), "up.sql");

        // get file 1
        RetrieveFileRequest retrieve_file_request;
        retrieve_file_request.set_file_id(obj1->id());
        const auto obj3 = file_service->RetrieveFile(retrieve_file_request);
        LOG_INFO("get returned: {}", obj3->ShortDebugString());
        ASSERT_EQ(obj3->id(), obj1->id());

        // download file 1
        DownloadFileRequest download_file_request;
        download_file_request.set_file_id(obj1->id());
        const auto content1 = file_service->DownloadFile(download_file_request);
        ASSERT_EQ(content1.value(), "helloworld");

        // list files
        ListFilesRequest list_files_request1;
        const auto list_response = file_service->ListFiles(list_files_request1);
        LOG_INFO("list returned: {}", list_response.ShortDebugString());
        ASSERT_EQ(list_response.object(), "list");
        ASSERT_EQ(list_response.data_size(), 2);
        util::MessageDifferencer differencer;
        ASSERT_TRUE(differencer.Compare(list_response.data(0), obj2.value()));
        ASSERT_TRUE(differencer.Compare(list_response.data(1), obj1.value()));
        // ASSERT_EQ(list_response.data(0).filename(), obj2->filename());
        // ASSERT_EQ(list_response.data(1).filename(), obj1->filename());
        ListFilesRequest list_files_request2;
        list_files_request2.set_purpose(assistants);
        const auto list_response2 = file_service->ListFiles(list_files_request2);
        ASSERT_EQ(list_response2.data_size(), 1);
        ASSERT_TRUE(differencer.Compare(list_response2.data(0), obj1.value()));

        // delete one file
        DeleteFileRequest delete_file_request;
        delete_file_request.set_file_id(obj1->id());
        const auto delete_reponse = file_service->DeleteFile(delete_file_request);
        LOG_INFO("delete returned: {}", delete_reponse.ShortDebugString());
        ASSERT_EQ(delete_reponse.id(), obj1->id());
        ASSERT_EQ(delete_reponse.object(), "file.deleted");
        ASSERT_EQ(delete_reponse.deleted(), true);

        // get file once again
        RetrieveFileRequest retrieve_file_request2;
        retrieve_file_request2.set_file_id(obj1->id());
        const auto obj4 = file_service->RetrieveFile(retrieve_file_request2);
        ASSERT_FALSE(obj4.has_value());
    }
}
