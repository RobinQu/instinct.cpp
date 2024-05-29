//
// Created by RobinQu on 2024/5/29.
//

#ifndef VECTORSTOREFILEBATCHDATAMAPPER_HPP
#define VECTORSTOREFILEBATCHDATAMAPPER_HPP

#include "database/IDataTemplate.hpp"
#include "AssistantGlobals.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;

    class VectorStoreFileBatchDataMapper final {
        DataTemplatePtr<VectorStoreObject, std::string> data_template_;

    public:
        std::optional<std::string> InsertVectorStoreFileBatch(const CreateVectorStoreFileBatchRequest &req) {

        }

        std::optional<VectorStoreFileBatch> GetVectorStoreFileBatch(const std::string& vs_store_id, const std::string& batch_id) {

        }

        size_t UpdateVectorStoreFileBatch(const std::string& vs_store_id, const std::string& batch_id, VectorStoreFileBatch_VectorStoreFileBatchStatus status) {

        }

    };

    using VectorStoreFileBatchDataMapperPtr = std::shared_ptr<VectorStoreFileBatchDataMapper>;
}


#endif //VECTORSTOREFILEBATCHDATAMAPPER_HPP
