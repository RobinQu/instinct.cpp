//
// Created by RobinQu on 2024/5/27.
//

#ifndef VECTORSTOREDATAMAPPER_HPP
#define VECTORSTOREDATAMAPPER_HPP

#include "database/IDataTemplate.hpp"
#include "AssistantGlobals.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;

    class VectorStoreDataMapper {
        DataTemplatePtr<VectorStoreObject, std::string> data_template_;
    public:
        size_t InsertVectorStore(const VectorStoreObject& obj) {

        }

        ListVectorStoresResponse ListVectorStores(const ListVectorStoresRequest& params) {
            ListVectorStoresResponse response;
            const auto list = data_template_->SelectMany("", {});
            response.mutable_data()->Add(list.begin(), list.end());
            if (!list.empty()) {
                response.set_first_id(list.front().id());
                response.set_last_id(list.back().id());
                response.set_object("list");
            }
            return response;
        }

        size_t UpdateVectorStore(const ModifyVectorStoreRequest& update) {

        }

        size_t DeleteVectorStore(const std::string& id) {

        }

        std::optional<VectorStoreObject> GetVectorStore(const std::string& id) {

        }

    };

    using VectorStoreDataMapperPtr = std::shared_ptr<VectorStoreDataMapper>;


}


#endif //VECTORSTOREDATAMAPPER_HPP
