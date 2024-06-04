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
            trace_span span {"FileObjectTaskHandler::Handle"};
            VectorStoreFileObject vs_file_object;
            ProtobufUtils::Deserialize(task.payload, vs_file_object);

            ModifyVectorStoreFileRequest modify_vector_store_file_request;
            modify_vector_store_file_request.set_vector_store_id(vs_file_object.vector_store_id());
            modify_vector_store_file_request.set_file_id(vs_file_object.file_id());
            modify_vector_store_file_request.set_status(in_progress);
            assert_true(vector_store_service_->ModifyVectorStoreFile(modify_vector_store_file_request), "should have VectorStoreFileObject updated");


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
                // OpenAI's parameters: https://github.com/RobinQu/instinct.cpp/issues/16#issuecomment-2133171030
                const auto splitter = CreateRecursiveCharacterTextSplitter({
                    .chunk_overlap = 400,
                    .chunk_size = 800
                });

                // load docs into vdb
                const auto retriever = retriever_operator_->GetStatefulRetriever(vs_object->id());
                retriever->Ingest(ingestor->LoadWithSplitter(splitter));

                // generate summary
                FindRequest find_request;
                find_request.mutable_query()->mutable_term()->set_name(METADATA_SCHEMA_PARENT_DOC_ID_KEY);
                find_request.mutable_query()->mutable_term()->mutable_term()->set_string_value(vs_file_object.file_id());
                const auto doc_itr = retriever->GetDocStore()->FindDocuments(find_request);
                const auto summary = CreateSummary(doc_itr, summary_chain_, reducible_fn_).get();
                modify_vector_store_file_request.set_summary(summary);
                LOG_DEBUG("summary for file {}: {}", vs_file_object.file_id(), summary);
                modify_vector_store_file_request.set_status(completed);
            } catch (...) {
                modify_vector_store_file_request.set_status(failed);
                modify_vector_store_file_request.mutable_last_error()->set_code(CommonErrorType::server_error);
                // TODO to print error log
                LOG_ERROR("File handling failed for {}", vs_file_object.ShortDebugString());
                modify_vector_store_file_request.mutable_last_error()->set_message("File handling failed");
            }

            vector_store_service_->ModifyVectorStoreFile(modify_vector_store_file_request);
        }

    private:
        void DownloadFile_(const FileObject& file_object, const TempFile& temp_file) const {
            std::ofstream output_stream {temp_file.path, std::ios::out | std::ios::trunc};
            DownloadFileRequest download_file_request;
            download_file_request.set_file_id(file_object.id());
            file_service_->DownloadFile(download_file_request, output_stream);
        }
    };
}

#endif //FILEINGESTIONTASKHANDLER_HPP
