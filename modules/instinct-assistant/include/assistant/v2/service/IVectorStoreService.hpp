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
        virtual ListVectorStoreFilesResponse ListVectorStoreFiles(const ListVectorStoreFilesRequest& req) = 0;
        virtual std::optional<VectorStoreFileObject> CreateVectorStoreFile(const CreateVectorStoreFileRequest& req) = 0;
        virtual std::optional<VectorStoreFileObject> GetVectorStoreFile(const GetVectorStoreFileRequest& req) = 0;
        virtual std::optional<VectorStoreFileObject> ModifyVectorStoreFile(const ModifyVectorStoreFileRequest& req) = 0;
        virtual DeleteVectorStoreFileResponse DeleteVectorStoreFile(const DeleteVectorStoreFileRequest& req) = 0;
        virtual std::optional<VectorStoreFileBatchObject> CreateVectorStoreFileBatche(const CreateVectorStoreFileBatchRequest& req) = 0;
        virtual std::optional<VectorStoreFileBatchObject> GetVectorStoreFileBatch(const GetVectorStoreFileBatchRequest& req) = 0;
        virtual std::optional<VectorStoreFileBatchObject> CancelVectorStoreFileBatch(const CancelVectorStoreFileBatchRequest& req) = 0;
        virtual ListFilesInVectorStoreBatchResponse ListFilesInVectorStoreBatch(const ListFilesInVectorStoreBatchRequest& req) = 0;


        // for internal use only
        virtual std::vector<VectorStoreFileObject> ListAllVectorStoreObjectFiles(const std::vector<std::string>& vector_store_ids) = 0;
        virtual std::vector<VectorStoreFileBatchObject> ListPendingFileBatcheObjects(size_t limit) = 0;
        virtual std::optional<VectorStoreFileBatchObject> ModifyVectorStoreFileBatch(const ModifyVectorStoreFileBatchRequest& req) = 0;
    };

    using VectorStoreServicePtr = std::shared_ptr<IVectorStoreService>;

}

#endif //VECSTORESERVICE_HPP
