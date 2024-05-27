//
// Created by RobinQu on 2024/4/19.
//

#ifndef VECSTORESERVICE_HPP
#define VECSTORESERVICE_HPP

#include <assistant_api_v2.pb.h>
#include "AssistantGlobals.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {

    class IVectorStoreService {
    public:
        IVectorStoreService()=default;
        virtual ~IVectorStoreService()=default;
        IVectorStoreService(IVectorStoreService&&)=delete;
        IVectorStoreService(const IVectorStoreService&)=delete;

        virtual ListVectorStoresResponse ListVectorStores(const ListVectorStoresRequest& req) = 0;
        virtual std::optional<VectorStoreObject> CreateVectorStore(const CreateVectorStoreRequest& req) = 0;
        virtual std::optional<VectorStoreObject> GetVectorStore(const GetVectorStoreRequest& req) = 0;
        virtual std::optional<VectorStoreObject> ModifyVectorStore(const ModifyVectorStoreRequest& req) = 0;
        virtual DeleteVectorStoreResponse DeleteVectorStore(const DeleteVectorStoreRequest& req) = 0;
        virtual ListVectorStoreFilesResponse ListVectorStoreFiles(const ListVectorStoresRequest& req) = 0;
        virtual std::optional<VectorStoreFileObject> CreateVectorStoreFile(const CreateVectorStoreFileRequest& req) = 0;
        virtual std::optional<VectorStoreFileObject> GetVectorStoreFile(const GetVectorStoreFileRequest& req) = 0;
        virtual DeleteVectorStoreFileResponse DeleteVectorStoreFile(const DeleteVectorStoreRequest& req) = 0;
    };

    using VectorStoreServicePtr = std::shared_ptr<IVectorStoreService>;

}

#endif //VECSTORESERVICE_HPP
