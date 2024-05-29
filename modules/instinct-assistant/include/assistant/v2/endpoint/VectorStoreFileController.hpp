//
// Created by RobinQu on 2024/4/19.
//

#ifndef VECTORSTOREFILECONTROLLER_HPP
#define VECTORSTOREFILECONTROLLER_HPP

#include "BaseController.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {

    class VectorStoreFileController final: public BaseController {
    public:
        explicit VectorStoreFileController(AssistantFacade facade)
            : BaseController(std::move(facade)) {
        }

        void Mount(HttpLibServer &server) override {
            server.GetRoute<ListVectorStoreFilesRequest, ListVectorStoreFilesResponse>("/v1/vector_stores/:vector_store_id/files", [&](ListVectorStoreFilesRequest& req, const HttpLibSession& session) {
                req.set_vector_store_id(session.request.path_params.at("vector_store_id"));
                const auto &resp = facade_.vector_store->ListVectorStoreFiles(req);
                session.Respond(resp);
            });

            server.PostRoute<CreateVectorStoreFileRequest, VectorStoreFileObject>("/v1/vector_stores/:vector_store_id/files", [&](CreateVectorStoreFileRequest& req, const HttpLibSession& session) {
                if (const auto &resp = facade_.vector_store->CreateVectorStoreFile(req)) {
                    session.Respond(resp.value());
                } else {
                    session.Respond("VectorStoreFile cannot be retrieved after creation", 500);
                }
            });

            server.GetRoute<GetVectorStoreFileRequest, VectorStoreFileObject>("/v1/vector_stores/:vector_store_id/files/:file_id", [&](GetVectorStoreFileRequest& req, const HttpLibSession& session) {
                req.set_vector_store_id(session.request.path_params.at("vector_store_id"));
                req.set_file_id(session.request.path_params.at("file_id"));
                if (const auto& resp = facade_.vector_store->GetVectorStoreFile(req)) {
                    session.Respond(resp.value());
                } else {
                    session.Respond(fmt::format("VectorStoreFile is not found with vector_store_id {} and file_id {}", req.vector_store_id(), req.file_id()));
                }
            });

            server.DeleteRoute<DeleteVectorStoreFileRequest, DeleteVectorStoreFileResponse>("/v1/vector_stores/:vector_store_id/:files/:file_id", [&](DeleteVectorStoreFileRequest& req, const HttpLibSession& session) {
                req.set_vector_store_id(session.request.path_params.at("vector_store_id"));
                req.set_file_id(session.request.path_params.at("file_id"));
                const auto& resp = facade_.vector_store->DeleteVectorStoreFile(req);
                session.Respond(resp);
            });

        }


    };


}



#endif //VECTORSTOREFILECONTROLLER_HPP
