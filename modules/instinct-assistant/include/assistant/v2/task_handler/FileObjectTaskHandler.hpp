//
// Created by RobinQu on 2024/5/27.
//

#ifndef FILEINGESTIONTASKHANDLER_HPP
#define FILEINGESTIONTASKHANDLER_HPP

#include <iostream>

#include "document/RecursiveCharacterTextSplitter.hpp"
#include "store/IVectorStoreOperator.hpp"
#include "task_scheduler/ThreadPoolTaskScheduler.hpp"
#include "AssistantGlobals.hpp"
#include "assistant/v2/service/IFileService.hpp"
#include "assistant/v2/tool/SimpleRetrieverOperator.hpp"
#include "ingestor/BaseIngestor.hpp"
#include "ingestor/DOCXFileIngestor.hpp"
#include "ingestor/PDFFileIngestor.hpp"
#include "ingestor/SingleFileIngestor.hpp"
#include "retrieval/BaseRetriever.hpp"
#include "retrieval/ChunkedMultiVectorRetriever.hpp"


#include "RetrieverObjectFactory.hpp"
#include "chain/SummaryChain.hpp"
#include "tools/file_vault/TempFile.hpp"


namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;
    using namespace INSTINCT_RETRIEVAL_NS;

    struct FileObjectTaskHandlerOptions {
        size_t summary_input_max_size = 8 * 1064;
    };

    class FileObjectTaskHandler final: public CommonTaskScheduler::ITaskHandler {
        RetrieverOperatorPtr retriever_operator_;
        VectorStoreServicePtr vector_store_service_;
        FileServicePtr file_service_;
        SummaryChainPtr summary_chain_;
        IsReducibleFn reducible_fn_;
        FileObjectTaskHandlerOptions options_;
    public:
        static inline std::string CATEGORY = "vector_store_file_object";

        FileObjectTaskHandler(
            RetrieverOperatorPtr retriever_operator,
            VectorStoreServicePtr vector_store_service,
            FileServicePtr file_service,
            SummaryChainPtr summary_chain,
            FileObjectTaskHandlerOptions options = {})
            : retriever_operator_(std::move(retriever_operator)),
              vector_store_service_(std::move(vector_store_service)),
              file_service_(std::move(file_service)),
              summary_chain_(std::move(summary_chain)),
              options_(std::move(options)){
            assert_true(summary_chain_);
            // TODO use tokenizer to calculate length
            reducible_fn_ = [&](const std::vector<std::string>& data, const std::string& delta) {
                size_t total = 0;
                for(const auto& t: data) {
                    total += t.size();
                }
                return total + delta.size() >= options_.summary_input_max_size;
            };
        }

        bool Accept(const ITaskScheduler<std::string>::Task &task) override {
            return task.category == CATEGORY;
        }

        void Handle(const ITaskScheduler<std::string>::Task &task) override {
            trace_span span {"FileObjectTaskHandler::Handle(" + task.task_id + ")"};
            VectorStoreFileObject vs_file_object;
            ProtobufUtils::Deserialize(task.payload, vs_file_object);

            if (!CheckVectorStoreFileOk(vs_file_object.vector_store_id(), vs_file_object.file_id())) {
                LOG_WARN("Abort for invalid state for VectorStoreFileObject");
                return;
            }

            ModifyVectorStoreFileRequest modify_vector_store_file_request;
            modify_vector_store_file_request.set_vector_store_id(vs_file_object.vector_store_id());
            modify_vector_store_file_request.set_file_id(vs_file_object.file_id());
            modify_vector_store_file_request.set_status(in_progress);
            assert_true(vector_store_service_->ModifyVectorStoreFile(modify_vector_store_file_request), "should have VectorStoreFileObject updated");
            LOG_INFO("VectorStoreFileObject is in_progress: {}", task.task_id);

            try {
                GetVectorStoreRequest get_vector_store_request;
                get_vector_store_request.set_vector_store_id(vs_file_object.vector_store_id());
                const auto vs_object = vector_store_service_->GetVectorStore(get_vector_store_request);
                assert_true(vs_object, "should have found VectorStoreObject for file");

                // get file object
                RetrieveFileRequest retrieve_file_request;
                retrieve_file_request.set_file_id(vs_file_object.file_id());
                const auto file_object = file_service_->RetrieveFile(retrieve_file_request);
                assert_true(file_object, fmt::format("should have found file object with VectorStoreFileObject {}", vs_file_object.ShortDebugString()));

                // build ingestor
                TempFile temp_file {file_object->filename()};
                DownloadFile_(file_object.value(), temp_file);
                const auto ingestor = RetrieverObjectFactory::CreateIngestor(
                    temp_file.path,
                    vs_file_object.file_id(),
                    [&](Document& document) {
                        auto* vs_id_field = document.add_metadata();
                        vs_id_field->set_name(VECTOR_STORE_ID_KEY);
                        vs_id_field->set_string_value(vs_file_object.vector_store_id());
                    }
                );
                assert_true(ingestor, "should have created ingestor for file");
                LOG_INFO("Ingestor is built: {}", task.task_id);
                // OpenAI's parameters: https://github.com/RobinQu/instinct.cpp/issues/16#issuecomment-2133171030
                const auto splitter = CreateRecursiveCharacterTextSplitter({
                    .chunk_size = 800,
                    .chunk_overlap = 400
                });

                // load docs into vdb
                LOG_INFO("Ingestor will start: {}", task.task_id);
                const auto retriever = retriever_operator_->GetStatefulRetriever(vs_object->id());
                retriever->Ingest(ingestor->LoadWithSplitter(splitter));
                LOG_INFO("Ingestor is finished: {}", task.task_id);

                // generate summary
                FindRequest find_request;
                find_request.mutable_query()->mutable_term()->set_name(METADATA_SCHEMA_PARENT_DOC_ID_KEY);
                find_request.mutable_query()->mutable_term()->mutable_term()->set_string_value(vs_file_object.file_id());
                const auto doc_itr = retriever->GetDocStore()->FindDocuments(find_request);
                LOG_INFO("Summary started: {}", task.task_id);
                const auto summary = CreateSummary(doc_itr, summary_chain_, reducible_fn_).get();
                modify_vector_store_file_request.set_summary(summary);
                LOG_INFO("Summary done {}: {}", vs_file_object.file_id(), summary);
                modify_vector_store_file_request.set_status(completed);
            } catch (...) {
                // TODO to print error log
                LOG_ERROR("File handling failed for {}", vs_file_object.ShortDebugString());
                modify_vector_store_file_request.set_status(failed);
                modify_vector_store_file_request.mutable_last_error()->set_code(CommonErrorType::server_error);
                modify_vector_store_file_request.mutable_last_error()->set_message("File handling failed");
            }

            if (!CheckVectorStoreFileOk(vs_file_object.vector_store_id(), vs_file_object.file_id())) {
                LOG_WARN("Abort for invalid state for VectorStoreFileObject");
                return;
            }


            // update file status
            assert_true(vector_store_service_->ModifyVectorStoreFile(modify_vector_store_file_request));

            // update file batch status
            if (StringUtils::IsNotBlankString(vs_file_object.file_batch_id())) {
                GetVectorStoreFileBatchRequest get_vector_store_file_batch_request;
                get_vector_store_file_batch_request.set_batch_id(vs_file_object.file_batch_id());
                get_vector_store_file_batch_request.set_vector_store_id(vs_file_object.vector_store_id());
                const auto file_batch_object = vector_store_service_->GetVectorStoreFileBatch(get_vector_store_file_batch_request);
                if (file_batch_object->file_counts().in_progress() == 0) {
                    ModifyVectorStoreFileBatchRequest modify_vector_store_file_batch_request;
                    modify_vector_store_file_batch_request.set_vector_store_id(file_batch_object->vector_store_id());
                    modify_vector_store_file_batch_request.set_batch_id(modify_vector_store_file_batch_request.batch_id());
                    if (file_batch_object->file_counts().cancelled() > 0) {
                        modify_vector_store_file_batch_request.set_status(VectorStoreFileBatchObject_VectorStoreFileBatchStatus_cancelled);
                    } else if (file_batch_object->file_counts().failed() > 0) {
                        modify_vector_store_file_batch_request.set_status(VectorStoreFileBatchObject_VectorStoreFileBatchStatus_failed);
                    } else if (file_batch_object->file_counts().completed() == file_batch_object->file_counts().total()) {
                        modify_vector_store_file_batch_request.set_status(VectorStoreFileBatchObject_VectorStoreFileBatchStatus_completed);
                    } else {
                        LOG_WARN("Invalid state for VectorStoreFileBatchObject: ", file_batch_object->ShortDebugString());
                    }
                    LOG_INFO("Updating file batch with request: {}", modify_vector_store_file_batch_request.ShortDebugString());
                    assert_true(vector_store_service_->ModifyVectorStoreFileBatch(modify_vector_store_file_batch_request));
                } else {
                    LOG_INFO("Not updating file batch as some files are still in progress");
                }
            }
        }

    private:

        [[nodiscard]] bool CheckVectorStoreFileOk(const std::string& vs_store_id, const std::string& file_id) const {
            GetVectorStoreFileRequest get_vector_store_file_request;
            get_vector_store_file_request.set_vector_store_id(vs_store_id);
            get_vector_store_file_request.set_file_id(file_id);
            const auto obj = vector_store_service_->GetVectorStoreFile(get_vector_store_file_request);
            return obj && obj->status() == in_progress;
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
