//
// Created by RobinQu on 2024/5/27.
//

#ifndef VECTORSTORESERVICEIMPL_HPP
#define VECTORSTORESERVICEIMPL_HPP

#include "../IVectorStoreService.hpp"
#include "assistant/v2/data_mapper/VectorStoreDataMapper.hpp"
#include "assistant/v2/data_mapper/VectorStoreFileDataMapper.hpp"
#include "assistant/v2/task_handler/FileObjectTaskHandler.hpp"
#include "assistant/v2/tool/RetrieverOperator.hpp"
#include "task_scheduler/ThreadPoolTaskScheduler.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {

    class VectorStoreServiceImpl final: public IVectorStoreService {
        VectorStoreFileDataMapperPtr vector_store_file_data_mapper_;
        VectorStoreDataMapperPtr vector_store_data_mapper_;
        CommonTaskSchedulerPtr task_scheduler_;
        RetrieverOperatorPtr retriever_operator_;

    public:
        VectorStoreServiceImpl(VectorStoreFileDataMapperPtr vector_store_file_data_mapper,
            VectorStoreDataMapperPtr vector_store_data_mapper, CommonTaskSchedulerPtr task_scheduler,
            RetrieverOperatorPtr retriever_operator)
            : vector_store_file_data_mapper_(std::move(vector_store_file_data_mapper)),
              vector_store_data_mapper_(std::move(vector_store_data_mapper)),
              task_scheduler_(std::move(task_scheduler)),
              retriever_operator_(std::move(retriever_operator)) {
        }

        ListVectorStoresResponse ListVectorStores(const ListVectorStoresRequest &req) override {
            trace_span span {"ListVectorStores"};
            return vector_store_data_mapper_->ListVectorStores(req);
        }

        std::optional<VectorStoreObject> CreateVectorStore(const CreateVectorStoreRequest &req) override {
            trace_span span {"CreateVectorStore"};
            VectorStoreObject vector_store_object;
            vector_store_object.set_id(details::generate_next_object_id("vs"));
            vector_store_object.set_name(req.name());
            assert_true(vector_store_data_mapper_->InsertVectorStore(vector_store_object) == 1, "should have object inserted");
            assert_true(retriever_operator_->ProvisionRetriever(vector_store_object), "db instance should be created");
            if(req.file_ids_size()>0) {
                vector_store_file_data_mapper_->InsertManyVectorStoreFiles(vector_store_object.id(), req.file_ids());

                if(task_scheduler_) {
                    // trigger background jobs for file objects
                    for(const auto files = vector_store_file_data_mapper_->ListVectorStoreFiles(vector_store_object.id(), req.file_ids()); const auto& file: files) {
                        task_scheduler_->Enqueue({
                            .task_id = vector_store_object.id(),
                            .category = FileObjectTaskHandler::CATEGORY,
                            .payload = ProtobufUtils::Serialize(file)
                        });
                    }
                }
            }
            return vector_store_data_mapper_->GetVectorStore(vector_store_object.id());
        }

        std::optional<VectorStoreObject> GetVectorStore(const GetVectorStoreRequest &req) override {
            auto vs = vector_store_data_mapper_->GetVectorStore(req.vector_store_id());
            if (!vs) {
                return std::nullopt;
            }
            // TODO need transaction
            const auto files = vector_store_file_data_mapper_->ListVectorStoreFiles(req.vector_store_id());
            const auto counts = vs->mutable_file_counts();
            std::map<VectorStoreFileStatus, int> statues;
            for(const auto& file: files) {
                statues[file.status()]++;
            }
            counts->set_completed(statues.contains(completed) ? statues.at(completed) : 0);
            counts->set_in_progress(statues.contains(in_progress) ? statues.at(in_progress) : 0);
            counts->set_failed(statues.contains(failed) ? statues.at(failed) : 0);
            counts->set_cancelled(statues.contains(cancelled) ? statues.at(cancelled) : 0);
            counts->set_total(files.size());
            return vs;
        }

        std::optional<VectorStoreObject> ModifyVectorStore(const ModifyVectorStoreRequest &req) override {
            assert_true(vector_store_data_mapper_->UpdateVectorStore(req) == 1, "should have vector object updated");
            GetVectorStoreRequest get_vector_store_request;
            get_vector_store_request.set_vector_store_id(req.vector_store_id());
            return GetVectorStore(get_vector_store_request);
        }

        DeleteVectorStoreResponse DeleteVectorStore(const DeleteVectorStoreRequest &req) override {
            // TODO need transaction
            const auto vector_store_object = vector_store_data_mapper_->GetVectorStore(req.vector_store_id());
            assert_true(vector_store_object, fmt::format("should have found VectorStoreObject with request {}", req.ShortDebugString()));
            const auto files = vector_store_file_data_mapper_->ListVectorStoreFiles(req.vector_store_id());
            const bool is_removable = std::ranges::all_of(files, [](const VectorStoreFileObject& file_object) {
               return file_object.status() != in_progress;
            });
            DeleteVectorStoreResponse response;
            response.set_id(req.vector_store_id());
            if (is_removable) {
                const auto file_ids = files | std::views::transform([](const VectorStoreFileObject& file) {
                    return file.id();
                });
                vector_store_file_data_mapper_->DeleteVectorStoreFiles(req.vector_store_id(), file_ids);
                vector_store_data_mapper_->DeleteVectorStore(req.vector_store_id());
                response.set_deleted(retriever_operator_->CleanupRetriever(vector_store_object.value()));
            } else {
                response.set_deleted(false);
            }
            return response;
        }

        ListVectorStoreFilesResponse ListVectorStoreFiles(const ListVectorStoresRequest &req) override {
            return vector_store_file_data_mapper_->ListVectorStoreFiles(req);
        }

        std::optional<VectorStoreFileObject> CreateVectorStoreFile(const CreateVectorStoreFileRequest &req) override {
            assert_true(vector_store_file_data_mapper_->InsertVectorStoreFile(req) == 1, "should have vector store file created");
            GetVectorStoreFileRequest get_request;
            get_request.set_vector_store_id(req.vector_store_id());
            get_request.set_file_id(req.file_id());
            const auto file_object = GetVectorStoreFile(get_request);
            if (task_scheduler_) {
                task_scheduler_->Enqueue({
                    .task_id = file_object->id(),
                    .category = FileObjectTaskHandler::CATEGORY,
                    .payload = ProtobufUtils::Serialize(file_object.value())
                });
            }
            return file_object;
        }

        std::optional<VectorStoreFileObject> GetVectorStoreFile(const GetVectorStoreFileRequest &req) override {
            return vector_store_file_data_mapper_->GetVectorStoreFile(req.vector_store_id(), req.file_id());
        }

        DeleteVectorStoreFileResponse DeleteVectorStoreFile(const DeleteVectorStoreRequest &req) override {
            const auto count = vector_store_file_data_mapper_->DeleteVectorStoreFile(req.vector_store_id(), req.file_id());
            DeleteVectorStoreFileResponse response;
            response.set_id(req.file_id());
            response.set_object("vector_store.file.deleted");
            response.set_deleted(count == 1);
            const auto vector_store_object = vector_store_data_mapper_->GetVectorStore(req.vector_store_id());
            const auto retriever = retriever_operator_->GetStatefulRetriever(vector_store_object.value());
            SearchQuery filter;
            auto* file_id_term = filter.mutable_term();
            file_id_term->set_name(VECTOR_STORE_FILE_ID_KEY);
            file_id_term->mutable_term()->set_string_value(req.file_id());
            retriever->Remove(filter);
            return response;
        }
    };
}


#endif //VECTORSTORESERVICEIMPL_HPP
