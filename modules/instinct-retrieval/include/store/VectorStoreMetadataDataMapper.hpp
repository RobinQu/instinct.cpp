//
// Created by RobinQu on 2024/5/27.
//

#ifndef VECTORSTOREINSTANCEMETADATAMAPPER_HPP
#define VECTORSTOREINSTANCEMETADATAMAPPER_HPP

#include "database/IDataTemplate.hpp"
#include "RetrievalGlobals.hpp"

namespace INSTINCT_RETRIEVAL_NS {

    namespace details {
        static void create_or_replace_instance_table(const DocStorePtr& metadata_db) {

        }
    }

    using namespace INSTINCT_DATA_NS;
    class VectorStoreMetadataDataMapper {
        DataTemplatePtr<VectorStoreInstanceMetadata, std::string> data_mapper_;
    public:
        explicit VectorStoreMetadataDataMapper(DataTemplatePtr<VectorStoreInstanceMetadata, std::string> data_mapper)
            : data_mapper_(std::move(data_mapper)) {
        }

        size_t InsertInstance(const VectorStoreInstanceMetadata& insert) {

        }

        std::vector<VectorStoreInstanceMetadata> ListInstances() {

        }

        size_t RemoveInstance(const std::string& instance_id) {

        }

        std::optional<VectorStoreInstanceMetadata> GetInstance(const std::string& instance_id) {

        }


    };

    using VectorStoreMeatdataDataMapperPtr = std::shared_ptr<VectorStoreMetadataDataMapper>;
}



#endif //VECTORSTOREINSTANCEMETADATAMAPPER_HPP
