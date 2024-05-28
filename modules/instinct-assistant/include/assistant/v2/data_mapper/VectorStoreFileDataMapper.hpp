//
// Created by RobinQu on 2024/5/27.
//

#ifndef VECTORSTOREFILEDATAMAPPER_HPP
#define VECTORSTOREFILEDATAMAPPER_HPP

#include "AssistantGlobals.hpp"
#include "database/IDataTemplate.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;

    class VectorStoreFileDataMapper {
        DataTemplatePtr<VectorStoreFileObject, std::string> data_template_;
    public:
        size_t InsertVectorStoreFile(const CreateVectorStoreFileRequest& create_vector_store_file_request) {

        }

        size_t InsertManyVectorStoreFiles(const std::string& vector_store_id, const RangeOf<std::string> auto & file_ids) {

        }

        ListVectorStoreFilesResponse ListVectorStoreFiles(const ListVectorStoresRequest &req) {

        }


        std::vector<VectorStoreFileObject> ListVectorStoreFiles(const std::string& vector_store_id, const RangeOf<std::string> auto& file_ids) {

        }

        std::vector<VectorStoreFileObject> ListVectorStoreFiles(const std::string& vector_store_id) {

        }

        size_t DeleteVectorStoreFile(const std::string& vector_store_id, const std::string& vector_store_file_id) {

        }

        size_t DeleteVectorStoreFiles(const std::string& vector_store_id, const RangeOf<std::string> auto& vector_store_file_ids) {

        }

        std::optional<VectorStoreFileObject> GetVectorStoreFile(const std::string& vector_store_id, const std::string& vector_store_file_id) {}
    };

    using VectorStoreFileDataMapperPtr = std::shared_ptr<VectorStoreFileDataMapper>;
}

#endif //VECTORSTOREFILEDATAMAPPER_HPP
