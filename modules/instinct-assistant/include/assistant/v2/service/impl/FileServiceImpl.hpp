//
// Created by RobinQu on 2024/4/25.
//

#ifndef FILESERVICEIMPL_HPP
#define FILESERVICEIMPL_HPP

#include "../IFileService.hpp"
#include "assistant/v2/tool/EntitySQLUtils.hpp"
#include "database/IDataMapper.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;

    class FileServiceImpl final: public IFileService {
        DataMapperPtr<FileObject, std::string> data_mapper_;
    public:
        ListFilesResponse ListFiles(const ListFilesRequest &list_files_request) override {

            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(list_files_request, context);
            const auto file_list = EntitySQLUtils::SelectManyFiles(data_mapper_, context);
            ListFilesResponse list_files_response;
            list_files_response.set_object("list");
            list_files_response.mutable_data()->Add(file_list.begin(), file_list.end());
            return list_files_response;
        }

        std::optional<FileObject> UploadFile(const UploadFileRequest &upload_file_request) override {
            assert_not_blank(upload_file_request.purpose(), "should provide purpose");
            // TODO save file
            SQLContext context;
            context["purpose"] = upload_file_request.purpose();
            const auto id = details::generate_next_object_id("file");;
            context["id"] = id;
            EntitySQLUtils::InsertOneFile(data_mapper_, context);
            RetrieveFileRequest retrieve_file_request;
            retrieve_file_request.set_file_id(id);
            return RetrieveFile(retrieve_file_request);
        }

        DeleteFileResponse DeleteFile(const DeleteFileRequest &delete_file_request) override {
            assert_not_blank(delete_file_request.file_id(), "should provide file_id");
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(delete_file_request, context);
            const auto count = EntitySQLUtils::DeleteFile(data_mapper_, context);
            DeleteFileResponse response;
            response.set_object("file");
            response.set_deleted(count == 1);
            response.set_id(delete_file_request.file_id());
            return response;
        }

        std::optional<FileObject> RetrieveFile(const RetrieveFileRequest &retrieve_file_request) override {
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(retrieve_file_request, context);
            return EntitySQLUtils::SelectOneFile(data_mapper_, context);
        }

        std::unique_ptr<std::byte[]> DownloadFile(const DownloadFileRequest &download_file_request) override {
            // TODO
            return {};
        }
    };
}

#endif //FILESERVICEIMPL_HPP
