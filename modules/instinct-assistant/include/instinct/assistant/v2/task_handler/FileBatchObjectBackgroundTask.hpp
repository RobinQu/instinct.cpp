//
// Created by RobinQu on 2024/6/10.
//

#ifndef FILEBATCHOBJECTBACKGROUNDTASK_HPP
#define FILEBATCHOBJECTBACKGROUNDTASK_HPP

#include "AssistantGlobals.hpp"
#include "assistant/v2/service/IVectorStoreService.hpp"
#include "ioc/ManagedApplicationContext.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {

    using namespace std::chrono_literals;

    /**
    * A temporary background task class
    */
    class IBackgroundTask {
    public:
        IBackgroundTask()=default;
        IBackgroundTask(IBackgroundTask&&)=delete;
        IBackgroundTask(const IBackgroundTask&)=delete;
        virtual ~IBackgroundTask()=default;

        virtual void Start()=0;
        virtual void Handle()=0;
        virtual void Stop()=0;
        virtual bool IsRunning()=0;
    };

    using BackgroundTaskPtr = std::shared_ptr<IBackgroundTask>;

    struct FileBatchObjectBackgroundTaskOptions {
        std::chrono::milliseconds interval_ = 10s;
        int scan_batch_size = 10;
    };

    class FileBatchObjectBackgroundTask final: public IBackgroundTask, public ILifeCycle {
        VectorStoreServicePtr vector_store_service_;
        RetrieverOperatorPtr retriever_operator_;
        std::thread thread_;
        volatile bool running_ = false;
        FileBatchObjectBackgroundTaskOptions options_;
    public:
        FileBatchObjectBackgroundTask(
            VectorStoreServicePtr vector_store_service,
            RetrieverOperatorPtr retriever_operator,
            FileBatchObjectBackgroundTaskOptions options = {})
            : vector_store_service_(std::move(vector_store_service)),
              retriever_operator_(std::move(retriever_operator)),
              options_(std::move(options)) {
              assert_gt(options_.interval_.count(), 100, "time interval should be at least 100ms");
              assert_gte(options_.scan_batch_size, 1, "scan_batch_size should be greater than or equal to one");
        }

        u_int32_t GetPriority() override {
            return LOWEST_PRIORITY;
        }

        bool IsRunning() override {
            return running_;
        }

        void Start() override {
            running_ = true;
            thread_ = std::thread([&] {
                while (running_) {
                    // sleep first for initial delay
                    std::this_thread::sleep_for(options_.interval_);
                    // handle wrapped by cpptrace
                    CPPTRACE_WRAP_BLOCK(
                        Handle();
                    );
                }
            });
            LOG_INFO("FileBatchObjectBackgroundTask started");
        }

        void Stop() override {
            if (running_) {
                LOG_DEBUG("FileBatchObjectBackgroundTask is shutting down.");
                running_ = false;
            }
            if (thread_.joinable()) {
                thread_.join();
                LOG_DEBUG("FileBatchObjectBackgroundTask exited");
            }
        }

        void Handle() override {
            ListPendingFileBatchObjectsRequest file_batch_objects_request;
            file_batch_objects_request.set_limit(options_.scan_batch_size);
            for (const auto& file_batch: vector_store_service_->ListPendingFileBatcheObjects(file_batch_objects_request)) {
                LOG_DEBUG("check file batch: {}", file_batch.ShortDebugString());
                ListFilesInVectorStoreBatchRequest req;
                req.set_batch_id(file_batch.id());
                req.set_vector_store_id(file_batch.vector_store_id());
                ListFilesInVectorStoreBatchResponse resp;
                resp.set_has_more(true);
                ModifyVectorStoreFileBatchRequest modify_vector_store_batch_request;
                modify_vector_store_batch_request.set_batch_id(file_batch.id());
                modify_vector_store_batch_request.set_vector_store_id(file_batch.vector_store_id());
                modify_vector_store_batch_request.set_sanity_check_at(ChronoUtils::GetCurrentEpochMicroSeconds());

                bool terminal = false;
                bool completed = true;
                const auto retriever = retriever_operator_->GetStatefulRetriever(file_batch.vector_store_id());
                assert_true(retriever, "should have found stateful retriever by vector store id: " + req.vector_store_id());
                // scan all files
                do {
                    // clean data in retriever for cancelled and failed files
                    SearchQuery search_query;
                    auto* terms_query = search_query.mutable_terms();
                    terms_query->set_name(METADATA_SCHEMA_FILE_SOURCE_KEY);

                    // trigger scan
                    resp = vector_store_service_->ListFilesInVectorStoreBatch(req);
                    for(const auto& file: resp.data()) {
                        if (file.status() == failed) {
                            LOG_DEBUG("Found failed file {} in file batch {}", file.file_id(), file.file_batch_id());
                            modify_vector_store_batch_request.set_status(VectorStoreFileBatchObject_VectorStoreFileBatchStatus_failed);
                            modify_vector_store_batch_request.mutable_last_error()->CopyFrom(file.last_error());
                            terminal = true;
                            completed = false;
                            terms_query->add_terms()->set_string_value(file.file_id());
                        }
                        if (file.status() == cancelled) {
                            LOG_DEBUG("Found canceled file {} in file batch {}", file.file_id(), file.file_batch_id());
                            modify_vector_store_batch_request.set_status(VectorStoreFileBatchObject_VectorStoreFileBatchStatus_cancelled);
                            terminal = true;
                            completed = false;
                            terms_query->add_terms()->set_string_value(file.file_id());
                        }
                        if (file.status() == in_progress) {
                            terminal = false;
                            completed = false;
                        }
                    }

                    // do cleanup
                    if (terms_query->terms_size()>0) {
                        retriever->Remove(search_query);
                    }
                } while (resp.has_more() && !terminal);

                if (completed) {
                    LOG_DEBUG("File batch is completed: {}", file_batch.ShortDebugString());
                    terminal = true;
                    modify_vector_store_batch_request.set_status(VectorStoreFileBatchObject_VectorStoreFileBatchStatus_completed);
                }

                if (terminal) {
                    LOG_DEBUG("File batch reached terminal state: {}", modify_vector_store_batch_request.ShortDebugString());
                    // update file batch object to refresh its timestamp
                    if (!vector_store_service_->ModifyVectorStoreFileBatch(modify_vector_store_batch_request)) {
                        LOG_ERROR("failed to update file batch: {}", modify_vector_store_batch_request.ShortDebugString());
                    }
                }
            }
        }
    };
}


#endif //FILEBATCHOBJECTBACKGROUNDTASK_HPP
