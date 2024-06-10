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
        DataTemplatePtr<VectorStoreFileBatchObject, std::string> data_template_;
    public:
        explicit VectorStoreFileBatchDataMapper(DataTemplatePtr<VectorStoreFileBatchObject, std::string> data_template)
            : data_template_(std::move(data_template)) {
        }

        [[nodiscard]] std::optional<std::string> InsertVectorStoreFileBatch(const CreateVectorStoreFileBatchRequest &req) const {
            SQLContext context;
            context["id"] = details::generate_next_object_id("vsfb");
            ProtobufUtils::ConvertMessageToJsonObject(req, context, {.keep_default_values = true});
            return data_template_->InsertOne(R"(
insert into instinct_vector_store_file_batch(id, vector_store_id, status) values(
    {{text(id)}},
    {{text(vector_store_id)}},
    'in_progress'
) returning id;
)", context);
        }

        [[nodiscard]] std::optional<VectorStoreFileBatchObject> GetVectorStoreFileBatch(const std::string& vs_store_id, const std::string& batch_id) const {
            SQLContext context;
            context["vector_store_id"] = vs_store_id;
            context["id"] = batch_id;
            return data_template_->SelectOne(R"(
select * from instinct_vector_store_file_batch where id = {{text(id)}};
)", context);
        }

        [[nodiscard]] size_t UpdateVectorStoreFileBatch(const ModifyVectorStoreFileBatchRequest &req) const {
            assert_true(req.status() != VectorStoreFileBatchObject_VectorStoreFileBatchStatus_unknown_vector_store_file_batch_status, "should assign correct status for VectorStoreFileBatchObject");
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(req, context);
            return data_template_->Execute(R"(
update instinct_vector_store_file_batch
set
{% if exists("status") %}
    status = {{text(status)}},
{% endif %}
{% if exists("last_error") %}
    last_error = {{stringify(last_error)}},
{% endif %}
    modified_at = now()
where id = {{text(batch_id)}};
            )", context);
        }

        [[nodiscard]] std::vector<VectorStoreFileBatchObject> ListPendingFileBatchObjects(const std::vector<VectorStoreFileBatchObject_VectorStoreFileBatchStatus> &filtered_status,
            size_t limit) const {
            SQLContext context;
            assert_non_empty_range(filtered_status, "should have at least one status in filter");
            for (const auto& status: filtered_status) {
                assert_true(status!=VectorStoreFileBatchObject_VectorStoreFileBatchStatus_unknown_vector_store_file_batch_status, "should provide valid status in filter");
                context["filtered_status"].push_back(VectorStoreFileBatchObject_VectorStoreFileBatchStatus_Name(status));
            }
            context["limit"] = limit;
            return data_template_->SelectMany(R"(select * from instinct_vector_store_file_batch
where
    status in (
##for s in filtered_status
        {{text(s)}},
##endfor
    )
order by modified_at asc
limit {{limit}};)", context);
        }


    };

    using VectorStoreFileBatchDataMapperPtr = std::shared_ptr<VectorStoreFileBatchDataMapper>;

}


#endif //VECTORSTOREFILEBATCHDATAMAPPER_HPP
