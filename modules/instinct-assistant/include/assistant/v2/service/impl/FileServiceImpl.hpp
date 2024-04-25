//
// Created by RobinQu on 2024/4/25.
//

#ifndef FILESERVICEIMPL_HPP
#define FILESERVICEIMPL_HPP

#include <utility>

#include "../IFileService.hpp"
#include "assistant/v2/tool/EntitySQLUtils.hpp"
#include "database/IDataMapper.hpp"
#include "object_store/IObjectStore.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;

    struct FileServiceOptions {
        std::string bucket_name = "mini-assistant";
    };

    class FileServiceImpl final: public IFileService {
        DataMapperPtr<FileObject, std::string> data_mapper_;
        ObjectStorePtr object_store_;
        FileServiceOptions options_;

    public:
        FileServiceImpl(const DataMapperPtr<FileObject, std::string> &data_mapper, ObjectStorePtr object_store, FileServiceOptions options = {})
            : data_mapper_(data_mapper),
              object_store_(std::move(object_store)),
              options_(std::move(options)){
        }

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
            assert_true(upload_file_request.purpose() != 0, "should provide purpose");
            SQLContext context;
            context["purpose"] = upload_file_request.purpose();

            // generate id
            const auto id = details::generate_next_object_id("file");;
            context["id"] = id;

            // upload file
            const auto object_key = details::map_file_object_key(upload_file_request.purpose(), id);
            const auto status = object_store_->PutObject(options_.bucket_name, id, object_key);
            assert_status_ok(status);

            // write db
            EntitySQLUtils::InsertOneFile(data_mapper_, context);
            RetrieveFileRequest retrieve_file_request;
            retrieve_file_request.set_file_id(id);
            return RetrieveFile(retrieve_file_request);
        }

        DeleteFileResponse DeleteFile(const DeleteFileRequest &delete_file_request) override {
            assert_not_blank(delete_file_request.file_id(), "should provide file_id");

            // find file
            RetrieveFileRequest retrieve_file_request;
            retrieve_file_request.set_file_id(delete_file_request.file_id());
            const auto file = RetrieveFile(retrieve_file_request);
            if (!file.has_value()) {
                DeleteFileResponse response;
                response.set_object("file");
                response.set_deleted(false);
                response.set_id(delete_file_request.file_id());
                return response;
            }

            // delete in db
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(delete_file_request, context);
            const auto count = EntitySQLUtils::DeleteFile(data_mapper_, context);

            // delete in object store
            const auto object_key = details::map_file_object_key(file->purpose(), file->id());
            const auto status = object_store_->DeleteObject(options_.bucket_name, object_key);
            assert_status_ok(status);

            DeleteFileResponse response;
            response.set_object("file");
            response.set_deleted(count == 1);
            response.set_id(file->id());
            return response;
        }

        std::optional<FileObject> RetrieveFile(const RetrieveFileRequest &retrieve_file_request) override {
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(retrieve_file_request, context);
            return EntitySQLUtils::SelectOneFile(data_mapper_, context);
        }

        std::optional<std::string> DownloadFile(const DownloadFileRequest &download_file_request) override {
            RetrieveFileRequest retrieve_file_request;
            retrieve_file_request.set_file_id(download_file_request.file_id());
            const auto file = RetrieveFile(retrieve_file_request);
            if (!file.has_value()) {
                LOG_WARN("Attempted to download non-existing file: bucket={}, file_id={}", options_.bucket_name, download_file_request.file_id());
                return std::nullopt;
            }
            const auto object_key = details::map_file_object_key(file->purpose(), file->id());
            std::string buf;
            object_store_->GetObject(options_.bucket_name, object_key, buf);
            return buf;
        }

    };
}

#endif //FILESERVICEIMPL_HPP