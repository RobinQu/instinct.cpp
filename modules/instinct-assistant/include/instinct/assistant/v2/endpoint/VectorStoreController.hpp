//
// Created by RobinQu on 2024/4/19.
//

#ifndef VECTORSTORECONTROLLER_HPP
#define VECTORSTORECONTROLLER_HPP

#include <instinct/assistant/v2/endpoint/BaseController.hpp>

namespace INSTINCT_ASSISTANT_NS::v2 {

    class VectorStoreController final: public BaseController {
    public:
        explicit VectorStoreController(AssistantFacade facade)
            : BaseController(std::move(facade)) {
        }

        void Mount(HttpLibServer &server) override {
            server.GetRoute<ListVectorStoresRequest, ListVectorStoresResponse>("/v1/vector_stores", [&](const ListVectorStoresRequest& req, const HttpLibSession& session) {
                session.Respond(facade_.vector_store->ListVectorStores(req));
            });

            server.PostRoute<CreateVectorStoreRequest, VectorStoreObject>("/v1/vector_stores", [&](const CreateVectorStoreRequest& req, const HttpLibSession& session) {
                if (const auto &resp = facade_.vector_store->CreateVectorStore(req)) {
                    session.Respond(resp.value());
                } else {
                    session.Respond("VectorStore is not retrieved after creation", 500);
                }
            });

            server.GetRoute<GetVectorStoreRequest, VectorStoreObject>("/v1/vector_stores/:vector_store_id", [&](GetVectorStoreRequest& req, const HttpLibSession& session) {
                req.set_vector_store_id(session.request.path_params.at("vector_store_id"));
                if (const auto &resp = facade_.vector_store->GetVectorStore(req)) {
                    session.Respond(resp.value());
                } else {
                    session.Respond(fmt::format("VectorStore is not found with vector_store_id {}", req.vector_store_id()), 404);
                }
            });

            server.PostRoute<ModifyVectorStoreRequest, VectorStoreObject>("/v1/vector_stores/:vector_store_id", [&](ModifyVectorStoreRequest& req, const HttpLibSession& session) {
                req.set_vector_store_id(session.request.path_params.at("vector_store_id"));
                if (const auto &resp = facade_.vector_store->ModifyVectorStore(req)) {
                    session.Respond(resp.value());
                } else {
                    session.Respond("VectorStore is not retrieved after creation", 500);
                }
            });

            server.DeleteRoute<DeleteVectorStoreRequest, DeleteVectorStoreResponse>("/v1/vector_stores/:vector_store_id", [&](DeleteVectorStoreRequest& req, const HttpLibSession& session) {
                req.set_vector_store_id(session.request.path_params.at("vector_store_id"));
                const auto& resp = facade_.vector_store->DeleteVectorStore(req);
                session.Respond(resp);
            });

        }


    };
}



#endif //VECTORSTORECONTROLLER_HPP
