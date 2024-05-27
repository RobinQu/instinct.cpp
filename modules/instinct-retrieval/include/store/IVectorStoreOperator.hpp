//
// Created by RobinQu on 2024/5/27.
//

#ifndef IVECTORSTORECONTROLLER_HPP
#define IVECTORSTORECONTROLLER_HPP

#include "IVectorStore.hpp"
#include "RetrievalGlobals.hpp"

namespace INSTINCT_RETRIEVAL_NS {

    class IVectorStoreOperator {
    public:
        IVectorStoreOperator()=default;
        virtual ~IVectorStoreOperator()=default;
        IVectorStoreOperator(const IVectorStoreOperator&)=delete;
        IVectorStoreOperator(IVectorStoreOperator&&)=delete;

        virtual VectorStorePtr CreateInstance(const std::string& instance_id) = 0;
        virtual VectorStorePtr CreateInstance(const std::string& instance_id, MetadataSchemaPtr metadata_schema) = 0;
        virtual std::optional<VectorStorePtr> LoadInstance(const std::string& instance_id)=0;
        virtual std::vector<std::string> ListInstances() = 0;
        virtual bool RemoveInstance(const std::string& instance_id) = 0;
    };

    using VectorStoreOperatorPtr = std::shared_ptr<IVectorStoreOperator>;
}


#endif //IVECTORSTORECONTROLLER_HPP
