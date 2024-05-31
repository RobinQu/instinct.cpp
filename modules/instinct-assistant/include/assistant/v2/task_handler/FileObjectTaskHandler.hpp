//
// Created by RobinQu on 2024/5/27.
//

#ifndef FILEINGESTIONTASKHANDLER_HPP
#define FILEINGESTIONTASKHANDLER_HPP

#include <document/RecursiveCharacterTextSplitter.hpp>

#include "store/IVectorStoreOperator.hpp"
#include "task_scheduler/ThreadPoolTaskScheduler.hpp"
#include "AssistantGlobals.hpp"
#include "assistant/v2/service/IFileService.hpp"
#include "assistant/v2/tool/RetrieverOperator.hpp"
#include "ingestor/BaseIngestor.hpp"
#include "ingestor/DOCXFileIngestor.hpp"
#include "ingestor/PDFFileIngestor.hpp"
#include "ingestor/SingleFileIngestor.hpp"
#include "retrieval/BaseRetriever.hpp"
#include "retrieval/ChunkedMultiVectorRetriever.hpp"
#include <iostream>

#include "tools/file_vault/TempFile.hpp"


namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;
    using namespace INSTINCT_RETRIEVAL_NS;

    struct FileObjectTaskHandlerOptions {

    };

    class FileObjectTaskHandler final: public CommonTaskScheduler::ITaskHandler {
        RetrieverOperatorPtr retriever_operator_;
        VectorStoreServicePtr vector_store_service_;
        FileServicePtr file_service_;
        FileObjectTaskHandlerOptions options_;

    public:
        FileObjectTaskHandler(RetrieverOperatorPtr retriever_operator, VectorStoreServicePtr vector_store_service, FileServicePtr file_service, FileObjectTaskHandlerOptions options)
            : retriever_operator_(std::move(retriever_operator)),
              vector_store_service_(std::move(vector_store_service)),
              file_service_(std::move(file_service)),
              options_(std::move(options)){
        }

        static inline std::string CATEGORY = "run_object";

        bool Accept(const ITaskScheduler<std::string>::Task &task) override {
            return task.category == CATEGORY;
        }

        void Handle(const ITaskScheduler<std::string>::Task &task) override {
            trace_span span {"FileObjectTaskHandler::Handle"};
            VectorStoreFileObject file_object;
            ProtobufUtils::Deserialize(task.payload, file_object);

            ModifyVectorStoreFileRequest modify_vector_store_file_request;
            modify_vector_store_file_request.set_vector_store_id(file_object.vector_store_id());
            modify_vector_store_file_request.set_file_id(file_object.file_id());
            modify_vector_store_file_request.set_status(in_progress);
            assert_true(vector_store_service_->ModifyVectorStoreFile(modify_vector_store_file_request), "should have VectorStoreFileObject updated");

            try {
                GetVectorStoreRequest get_vector_store_request;
                get_vector_store_request.set_vector_store_id(file_object.vector_store_id());
                const auto vs_object = vector_store_service_->GetVectorStore(get_vector_store_request);
                assert_true(vs_object, "should have found VectorStoreObject for file");

                const auto retriever = retriever_operator_->GetStatefulRetriever(vs_object.value());
                TempFile temp_file;
                const auto ingestor = BuildIngestor_(file_object, temp_file);
                assert_true(ingestor, "should have created ingestor for file");
                retriever->Ingest(ingestor->Load());
            } catch (...) {
                modify_vector_store_file_request.set_status(failed);
                modify_vector_store_file_request.mutable_last_error()->set_code(CommonErrorType::server_error);
                // TODO to print error log
                LOG_ERROR("File ingestion failed for {}", file_object.ShortDebugString());
                modify_vector_store_file_request.mutable_last_error()->set_message("File ingestion failed");
                vector_store_service_->ModifyVectorStoreFile(modify_vector_store_file_request);
            }

            modify_vector_store_file_request.set_status(completed);
            vector_store_service_->ModifyVectorStoreFile(modify_vector_store_file_request);
        }

    private:
        [[nodiscard]] IngestorPtr BuildIngestor_(const VectorStoreFileObject& vs_file_object, const TempFile& temp_file) const {
            RetrieveFileRequest retrieve_file_request;
            retrieve_file_request.set_file_id(vs_file_object.file_id());
            const auto file_object = file_service_->RetrieveFile(retrieve_file_request);
            assert_true(file_object, fmt::format("should have found file object with VectorStoreFileObject {}", vs_file_object.ShortDebugString()));
            static std::regex EXT_NAME_PATTERN { R"(.+\.(.+))"};
            // we rely on extname to choose ingestor
            if (std::smatch match; std::regex_match(file_object->filename(), match, EXT_NAME_PATTERN)) {
                if (match.size() == 2) {
                    const auto extname = StringUtils::ToLower(match[1].str());
                    if (extname == "pdf") {
                        DownloadFile_(file_object.value(), temp_file);
                        return CreatePDFFileIngestor(temp_file.path);
                    }
                    if (extname == "txt" || extname == "md") {
                        DownloadFile_(file_object.value(), temp_file);
                        return CreatePlainTextFileIngestor(temp_file.path);
                    }
                    if (extname == "docx") {
                        DownloadFile_(file_object.value(), temp_file);
                        return CreateDOCXFileIngestor(temp_file.path);
                    }
                }
            }
            LOG_WARN("Ingestor not built for file object: {}", file_object->ShortDebugString());
            return nullptr;
        }

        void DownloadFile_(const FileObject& file_object, const TempFile& temp_file) const {
            std::ofstream output_stream {temp_file.path, std::ios::out | std::ios::trunc};
            DownloadFileRequest download_file_request;
            download_file_request.set_file_id(file_object.id());
            file_service_->DownloadFile(download_file_request, output_stream);
        }
    };
}

#endif //FILEINGESTIONTASKHANDLER_HPP
