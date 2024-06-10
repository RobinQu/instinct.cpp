//
// Created by RobinQu on 2024/6/10.
//

#ifndef FILEBATCHOBJECTBACKGROUNDTASK_HPP
#define FILEBATCHOBJECTBACKGROUNDTASK_HPP

#include "AssistantGlobals.hpp"
#include "assistant/v2/service/IVectorStoreService.hpp"

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
        virtual void Shutdown()=0;
        virtual bool isRunning()=0;
    };

    using BackgroundTaskPtr = std::shared_ptr<IBackgroundTask>;

    struct FileBatchObjectBackgroundTaskOptions {
        std::chrono::milliseconds interval_ = 10s;
        size_t scan_batch_size = 10;
    };

    class FileBatchObjectBackgroundTask final: public IBackgroundTask {
        VectorStoreServicePtr vector_store_service_;
        std::thread thread_;
        volatile bool running_ = false;
        FileBatchObjectBackgroundTaskOptions options_;
    public:
        explicit FileBatchObjectBackgroundTask(VectorStoreServicePtr vector_store_service,
            FileBatchObjectBackgroundTaskOptions options = {})
            : vector_store_service_(std::move(vector_store_service)),
              options_(std::move(options)) {
              assert_gt(options_.interval_.count(), 100, "time interval should be at least 100ms");
              assert_gte(options_.scan_batch_size, 1, "scan_batch_size should be greater than or equal to one");
        }

        ~FileBatchObjectBackgroundTask() override {
            LOG_DEBUG("Shutting dow FileBatchObjectBackgroundTask");
            Shutdown();
        }

        bool isRunning() override {
            return running_;
        }

        void Start() override {
            running_ = true;
            thread_ = std::thread([&] {
                while (running_) {
                    CPPTRACE_WRAP_BLOCK(
                        Handle();
                    );
                    std::this_thread::sleep_for(options_.interval_);
                }
            });
        }

        void Shutdown() override {
            running_ = false;
            if (thread_.joinable()) {
                thread_.join();
            }
        }

        void Handle() override {
            for (const auto& file_batch: vector_store_service_->ListPendingFileBatcheObjects(options_.scan_batch_size)) {
                LOG_DEBUG("check file batch: {}", file_batch.ShortDebugString());
                ListFilesInVectorStoreBatchRequest req;
                req.set_batch_id(file_batch.id());
                ListFilesInVectorStoreBatchResponse resp;
                resp.set_has_more(true);
                ModifyVectorStoreFileBatchRequest modify_vector_store_batch_request;
                modify_vector_store_batch_request.set_batch_id(file_batch.id());

                bool terminal = false;
                bool completed = true;
                // scan all files
                do {
                    resp = vector_store_service_->ListFilesInVectorStoreBatch(req);
                    for(const auto& file: resp.data()) {
                        if (file.status() == failed) {
                            LOG_DEBUG("Found failed file {} in file batch {}", file.file_id(), file.file_batch_id());
                            modify_vector_store_batch_request.set_status(VectorStoreFileBatchObject_VectorStoreFileBatchStatus_failed);
                            modify_vector_store_batch_request.mutable_last_error()->CopyFrom(file.last_error());
                            terminal = true;
                            completed = false;
                        }
                        if (file.status() == cancelled) {
                            LOG_DEBUG("Found canceled file {} in file batch {}", file.file_id(), file.file_batch_id());
                            modify_vector_store_batch_request.set_status(VectorStoreFileBatchObject_VectorStoreFileBatchStatus_cancelled);
                            terminal = true;
                            completed = false;
                        }
                        if (file.status() == in_progress) {
                            terminal = false;
                            completed = false;
                        }
                    }
                } while (resp.has_more() && !terminal);

                if (completed) {
                    LOG_DEBUG("File batch is completed: {}", file_batch.ShortDebugString());
                    terminal = true;
                    modify_vector_store_batch_request.set_status(VectorStoreFileBatchObject_VectorStoreFileBatchStatus_completed);
                }

                if (terminal) {
                    LOG_DEBUG("File batch reached terminal state: {}", modify_vector_store_batch_request.ShortDebugString());
                }

                // always update file batch object to refresh its timestamp
                if (!vector_store_service_->ModifyVectorStoreFileBatch(modify_vector_store_batch_request)) {
                    LOG_ERROR("failed to update file batch: {}", modify_vector_store_batch_request.ShortDebugString());
                }

            }
        }
    };
}


#endif //FILEBATCHOBJECTBACKGROUNDTASK_HPP
