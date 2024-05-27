//
// Created by RobinQu on 2024/5/27.
//

#ifndef FILEINGESTIONTASKHANDLER_HPP
#define FILEINGESTIONTASKHANDLER_HPP

#include <document/RecursiveCharacterTextSplitter.hpp>

#include "store/IVectorStoreOperator.hpp"
#include "task_scheduler/ThreadPoolTaskScheduler.hpp"
#include "AssistantGlobals.hpp"
#include "ingestor/BaseIngestor.hpp"
#include "ingestor/DOCXFileIngestor.hpp"
#include "ingestor/PDFFileIngestor.hpp"
#include "ingestor/SingleFileIngestor.hpp"
#include "retrieval/BaseRetriever.hpp"
#include "retrieval/ChunkedMultiVectorRetriever.hpp"


namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;
    using namespace INSTINCT_RETRIEVAL_NS;

    struct FileObjectTaskHandlerOptions {
        int parent_chunk_size = 1000;
        int child_chunk_size = 200;
    };

    class FileObjectTaskHandler final: public CommonTaskScheduler::ITaskHandler {
        static inline std::string CATEGORY = "run_object";

        VectorStoreOperatorPtr vector_store_operator_;
        FileObjectTaskHandlerOptions options_;

    public:
        FileObjectTaskHandler(VectorStoreOperatorPtr vector_store_operator, FileObjectTaskHandlerOptions options)
            : vector_store_operator_(std::move(vector_store_operator)),
              options_(std::move(options)) {
              assert_true(options_.parent_chunk_size > options_.child_chunk_size, "parent_chunk_size should be larger than child_chunk_size");
        }


        bool Accept(const ITaskScheduler<std::string>::Task &task) override {
            return task.category == CATEGORY;
        }

        void Handle(const ITaskScheduler<std::string>::Task &task) override {
            trace_span span {"FileObjectTaskHandler::Handle"};
            FileObject file_object;
            ProtobufUtils::Deserialize(task.payload, file_object);

            if (const auto existing_instance = vector_store_operator_->LoadInstance(file_object.id())) {
                LOG_WARN("file found in vdb. task aborted. file_object={}", file_object.ShortDebugString());
                return;
            }

            const auto retriever = BuildRetriever_(file_object);
            const auto ingestor = BuildIngestor_(file_object);
            retriever->Ingest(ingestor->Load());

        }

    private:
        [[nodiscard]] StatefulRetrieverPtr BuildRetriever_(const FileObject& file_object) const {
            const auto vector_store = vector_store_operator_->CreateInstance(file_object.id());
            const auto tokenizer = TiktokenTokenizer::MakeGPT4Tokenizer();
            const auto child_spliter = CreateRecursiveCharacterTextSplitter(tokenizer, {.chunk_size = options_.child_chunk_size});
            const auto doc_store = vector_store->AsDocStore();
            if (options_.parent_chunk_size > 0) {
                const auto parent_splitter = CreateRecursiveCharacterTextSplitter({.chunk_size = options_.parent_chunk_size});
                return CreateChunkedMultiVectorRetriever(
                    doc_store,
                    vector_store,
                    child_spliter,
                    parent_splitter
                );
            }
            return CreateChunkedMultiVectorRetriever(
                doc_store,
                vector_store,
                child_spliter
            );
        }

        IngestorPtr BuildIngestor_(const FileObject& file_object) {
            static std::regex EXT_NAME_PATTERN { R"(.+\.(.+))"};
            // we rely on extname to choose ingestor
            if (std::smatch match; std::regex_match(file_object.filename(), match, EXT_NAME_PATTERN)) {
                if (match.size() == 2) {
                    const auto extname = StringUtils::ToLower(match[1].str());
                    if (extname == "pdf") {
                        return CreatePDFFileIngestor(CreateTempFile_(file_object));
                    }
                    if (extname == "txt" || extname == "md") {
                        return CreatePlainTextFileIngestor(CreateTempFile_(file_object));
                    }
                    if (extname == "docx") {
                        return CreateDOCXFileIngestor(CreateTempFile_(file_object));
                    }
                }
            }
            LOG_WARN("Ingestor not built for file object: {}", file_object.ShortDebugString());
            return nullptr;
        }

        std::filesystem::path CreateTempFile_(const FileObject& file_object) {

        }
    };
}

#endif //FILEINGESTIONTASKHANDLER_HPP
