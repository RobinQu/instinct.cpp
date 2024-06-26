//
// Created by RobinQu on 2024/4/19.
//

#ifndef FILECONTROLLER_HPP
#define FILECONTROLLER_HPP


#include <instinct/assistant/v2/endpoint/BaseController.hpp>
#include <instinct/tools/file_vault/temp_file.hpp>

namespace INSTINCT_ASSISTANT_NS::v2 {

    class FileController final: public BaseController {

    public:
        explicit FileController(const AssistantFacade &facade)
            : BaseController(facade) {
        }

        void Mount(HttpLibServer &server) override {
            server.GetRoute<ListFilesRequest, ListFilesResponse>("/v1/files", [&](ListFilesRequest& req, const HttpLibSession& session) {
                if (session.request.has_param("purpose") && StringUtils::IsNotBlankString(session.request.get_param_value("purpose"))) {
                    FileObjectPurpose purpose;
                    FileObjectPurpose_Parse(session.request.get_param_value("purpose"), &purpose);
                    req.set_purpose(purpose);
                }
                const auto resp = facade_.file->ListFiles(req);
                session.Respond(resp);
            });


            server.GetRoute<RetrieveFileRequest, FileObject>("/v1/files/:file_id", [&](RetrieveFileRequest& req, const HttpLibSession& session) {
                req.set_file_id(session.request.path_params.at("file_id"));
                const auto v = facade_.file->RetrieveFile(req);
                if (v.has_value()) {
                    session.Respond(v.value());
                } else {
                    session.Respond(fmt::format("No file found with id '{}'", req.file_id()), 404);
                }
            });

            server.GetRoute<DownloadFileRequest, std::string>("/v1/files/:file_id/content", [&](DownloadFileRequest& req, const HttpLibSession& session) {
                req.set_file_id(session.request.path_params.at("file_id"));
                const auto buf = facade_.file->DownloadFile(req);
                if (buf.has_value()) {
                    return session.Respond(buf.value());
                }
                session.Respond(fmt::format("No file found with id '{}'", req.file_id()), 404);
            });

            server.DeleteRoute<DeleteFileRequest, DeleteFileResponse>("/v1/files/:file_id", [&](DeleteFileRequest& req, const HttpLibSession& session) {
                req.set_file_id(session.request.path_params.at("file_id"));
                const auto resp = facade_.file->DeleteFile(req);
                session.Respond(resp);
            });

            // upload endpoint is accepting multipart-formdata, not Restful
            server.GetHttpLibServer().Post("/v1/files", [&](const Request& req, Response& res, const ContentReader &content_reader) {

                if (!req.is_multipart_form_data()) {
                    return HttpLibSession::Respond(res, "Only multipart-form is accpted at this endpoint", 400);
                }

                // NOTE: `content_reader` is blocking until every form data field is read
                std::unordered_map<std::string, TempFile> files;
                std::vector<std::string> file_names;
                std::vector<std::string> content_types;
                std::vector<std::string> field_names;
                std::unordered_map<std::string, std::string> params;
                bool is_file = false;
                content_reader(
                [&](const MultipartFormData &field) {
                    field_names.push_back(field.name);
                    if (StringUtils::IsNotBlankString(field.filename)) { // treat as file object
                        is_file = true;
                        const auto temp_file = std::filesystem::temp_directory_path() / StringUtils::GenerateUUIDString();
                        files.emplace(field.name, TempFile {});
                        file_names.emplace_back(field.filename);
                        content_types.emplace_back(field.content_type);
                    } else {
                        is_file = false;
                        params[field.name] = "";
                    }
                    return true;
                },
                [&](const char *data, const size_t data_length) {
                    if (is_file && files.contains(field_names.back())) {
                        files[field_names.back()].file.write(data, data_length);
                    } else {
                        params[field_names.back()] += std::string {data, data_length};
                    }
                    return true;
                });

                if (files.empty()) {
                    return HttpLibSession::Respond(res, "No file is not included", 400);
                }

                if (!files.contains("file")) {
                    return HttpLibSession::Respond(res, "No file object named 'file' is found in multipart form data.", 400);
                }

                if (!params.contains("purpose")) {
                    return HttpLibSession::Respond(res, "Purpose of file is not provided", 400);
                }

                FileObjectPurpose file_object_purpose;
                if (!FileObjectPurpose_Parse(params.at("purpose"), &file_object_purpose)) {
                    return HttpLibSession::Respond(res, "Illegal purpose string of file is not provided: " + params.at("purpose"), 400);
                }

                // only accept first file
                UploadFileRequest upload_file_request;
                upload_file_request.set_filename(file_names[0]);
                upload_file_request.set_purpose(file_object_purpose);

                // reset flag for reading
                auto& file = files.at("file").file;
                file.clear();
                file.seekg(0);

                // send file to the file service
                if (const auto v = facade_.file->UploadFile(upload_file_request, files.at("file").file)) {
                    ProtobufUtils::Serialize(v.value(), res.body);
                    res.status = 200;
                } else {
                    HttpLibSession::Respond(res, "File is not retrieved after uploaded", 500);
                }
            });
        }
    };
}



#endif //FILECONTROLLER_HPP
