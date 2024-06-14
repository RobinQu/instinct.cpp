//
// Created by RobinQu on 2024/4/19.
//

#ifndef VECTORSTOREFILEBATCHCONTROLLER_HPP
#define VECTORSTOREFILEBATCHCONTROLLER_HPP

#include "BaseController.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {

    class VectorStoreFileBatchController final: public BaseController {
    public:
        explicit VectorStoreFileBatchController(AssistantFacade facade)
            : BaseController(std::move(facade)) {
        }

        void Mount(HttpLibServer &server) override {
            server.PostRoute<CreateVectorStoreFileBatchRequest, VectorStoreFileBatchObject>("/v1/vector_stores/:vector_store_id/file_batches", [&](CreateVectorStoreFileBatchRequest& req, const HttpLibSession& session) {
                req.set_vector_store_id(session.request.path_params.at("vector_store_id"));
                if (const auto &resp = facade_.vector_store->CreateVectorStoreFileBatch(req)) {
                    session.Respond(resp.value());
                } else {
                    session.Respond("VectorStoreFileBatch cannot be retrieved after creation", 500);
                }
            });

            server.GetRoute<GetVectorStoreFileBatchRequest, VectorStoreFileBatchObject>("/v1/vector_stores/:vector_store_id/file_batches/:batch_id", [&](GetVectorStoreFileBatchRequest& req, const HttpLibSession& session) {
                req.set_vector_store_id(session.request.path_params.at("vector_store_id"));
                req.set_batch_id(session.request.path_params.at("batch_id"));
                if(const auto &resp = facade_.vector_store->GetVectorStoreFileBatch(req)) {
                    session.Respond(resp.value());
                } else {
                    session.Respond(fmt::format("VectorStoreFileBatch cannot be found with vector_store_id {} and batch_id {}", req.vector_store_id(), req.batch_id()), 404);
                }
            });


            server.PostRoute<CancelVectorStoreFileBatchRequest, VectorStoreFileBatchObject>("/v1/vector_stores/:vector_store_id/file_batches/:batch_id/cancel", [&](CancelVectorStoreFileBatchRequest& req, const HttpLibSession& session) {
                req.set_vector_store_id(session.request.path_params.at("vector_store_id"));
                req.set_batch_id(session.request.path_params.at("batch_id"));
                if (const auto& resp = facade_.vector_store->CancelVectorStoreFileBatch(req)) {
                    session.Respond(resp.value());
                } else {
                    session.Respond("VectorStoreFileBatch cannot be retrieved after cancellation", 500);
                }
            });

            server.GetRoute<ListFilesInVectorStoreBatchRequest, ListFilesInVectorStoreBatchResponse>("/v1/vector_stores/:vector_store_id/file_batches/:batch_id/files", [&](ListFilesInVectorStoreBatchRequest& req, const HttpLibSession& session) {
                req.set_vector_store_id(session.request.path_params.at("vector_store_id"));
                req.set_batch_id(session.request.path_params.at("batch_id"));
                const auto& resp = facade_.vector_store->ListFilesInVectorStoreBatch(req);
                session.Respond(resp);
            });

        }
    };
}



#endif //VECTORSTOREFILEBATCHCONTROLLER_HPP
