//
// Created by RobinQu on 2024/5/27.
//

#ifndef VECTORSTORESERVICEIMPL_HPP
#define VECTORSTORESERVICEIMPL_HPP

#include "../IVectorStoreService.hpp"
#include "assistant/v2/data_mapper/VectorStoreDataMapper.hpp"
#include "assistant/v2/data_mapper/VectorStoreFileDataMapper.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    class VectorStoreServiceImpl final: public IVectorStoreService {
        VectorStoreFileDataMapperPtr vector_store_file_data_mapper_;
        VectorStoreDataMapperPtr vector_store_data_mapper_;
    public:
        ListVectorStoresResponse ListVectorStores(const ListVectorStoresRequest &req) override {
            trace_span span {"ListVectorStores"};
            return vector_store_data_mapper_->ListVectorStores(req);
        }

        std::optional<VectorStoreObject> CreateVectorStore(const CreateVectorStoreRequest &req) override {
            VectorStoreObject vector_store_object;
            vector_store_object.set_id(details::generate_next_object_id("vs"));
            vector_store_object.set_name(req.name());
            assert_true(vector_store_data_mapper_->InsertVectorStore(vector_store_object) == 1, "should have object inserted");

            if(req.file_ids_size()>0) {
                vector_store_file_data_mapper_->InsertManyVectorStoreFiles(vector_store_object.id(), req.file_ids());
                // TODO background file handling
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
            // TODO check if there are any in progress jobs
            // TODO need transaction
            const auto files = vector_store_file_data_mapper_->ListVectorStoreFiles(req.vector_store_id());

            const auto file_ids = files | std::views::transform([](const VectorStoreFileObject& file) {
                return file.id();
            });
            const auto count = vector_store_file_data_mapper_->DeleteVectorStoreFiles(req.vector_store_id(), file_ids);

            DeleteVectorStoreResponse response;
            response.set_id(req.vector_store_id());
            response.set_deleted(count == 1);
            return response;
        }

        ListVectorStoreFilesResponse ListVectorStoreFiles(const ListVectorStoresRequest &req) override {
            return vector_store_file_data_mapper_->ListVectorStoreFiles(req);
        }

        std::optional<VectorStoreFileObject> CreateVectorStoreFile(const CreateVectorStoreFileRequest &req) override {
            assert_true(vector_store_file_data_mapper_->InsertVectorStoreFile(req) == 1, "should have vector store file created");
            // TODO trigger background job
            GetVectorStoreFileRequest get_request;
            get_request.set_vector_store_id(req.vector_store_id());
            get_request.set_file_id(req.file_id());
            return GetVectorStoreFile(get_request);
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
            return response;
        }
    };
}


#endif //VECTORSTORESERVICEIMPL_HPP
