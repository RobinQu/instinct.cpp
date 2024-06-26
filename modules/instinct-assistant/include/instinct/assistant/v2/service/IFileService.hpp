//
// Created by RobinQu on 2024/4/19.
//

#ifndef IFILESERVICE_HPP
#define IFILESERVICE_HPP

#include <assistant_api_v2.pb.h>
#include <instinct/AssistantGlobals.hpp>

namespace INSTINCT_ASSISTANT_NS::v2{
    class IFileService {
    public:
        IFileService()=default;
        virtual ~IFileService()=default;
        IFileService(IFileService&&)=delete;
        IFileService(const IFileService&)=delete;

        virtual ListFilesResponse ListFiles(const ListFilesRequest& list_files_request) = 0;
        virtual std::optional<FileObject> UploadFile(const UploadFileRequest& upload_file_request) = 0;
        virtual std::optional<FileObject> UploadFile(const UploadFileRequest& upload_file_request, std::istream& input_stream) = 0;
        virtual DeleteFileResponse DeleteFile(const DeleteFileRequest& delete_file_request) = 0;
        virtual std::optional<FileObject> RetrieveFile(const RetrieveFileRequest& retrieve_file_request) = 0;
        virtual std::optional<std::string> DownloadFile(const DownloadFileRequest& download_file_request) = 0;
        virtual void DownloadFile(const DownloadFileRequest& download_file_request, std::ostream& output_stream) = 0;
    };

    using FileServicePtr = std::shared_ptr<IFileService>;

}

#endif //IFILESERVICE_HPP
