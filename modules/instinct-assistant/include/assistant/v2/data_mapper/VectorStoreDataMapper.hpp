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
        explicit VectorStoreDataMapper(DataTemplatePtr<VectorStoreObject, std::string> data_template)
            : data_template_(std::move(data_template)) {
        }

        [[nodiscard]] std::optional<std::string> InsertVectorStore(const CreateVectorStoreRequest& req) const {
            VectorStoreObject vector_store_object;
            vector_store_object.set_id(details::generate_next_object_id("vs"));
            vector_store_object.set_name(req.name());
            if (req.has_expires_after()) {
                assert_true(req.expires_after().anchor() == VectorStoreExpirationPolicy::last_active_at, "should have anchor set to last_active_at");
                assert_gt(req.expires_after().days(), 0, "should have expires_after.days set with a positive value");
                vector_store_object.mutable_expires_after()->CopyFrom(req.expires_after());
            }
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(req, context, {.keep_default_values = true});
            return data_template_->InsertOne(R"(
insert into instinct_vector_store(name, file_counts, status, expires_after, expires_at, last_active_at) values(
    {{text(name)}},
    {{stringify(file_counts)}},
    {% if exists("expires_after") %}
    {{stringify(expires_after)}},
    {% else %}
    NULL,
    {% endif %}
    {% if exists("expires_at") and expires_at>0 %}
    {{timestamp(expires_at)}},
    {% else %}
    NULL,
    {% endif %}
    now()
) returning id;
)", context);
        }

        [[nodiscard]] ListVectorStoresResponse ListVectorStores(const ListVectorStoresRequest& params) const {
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(params, context, {.keep_default_values=true});
            // limit + 1 to check if there is more records that match the conditions
            auto limit = params.limit() <= 0 ? DEFAULT_LIST_LIMIT + 1 : params.limit() + 1;
            context["limit"] = limit;
            if (params.order() == unknown_list_request_order) {
                context["order"] = "desc";
            }
            const auto list = data_template_->SelectMany(R"(
select * from instinct_vector_store
where 1=1
    {% if is_not_blank(after) %}
    and id > {{text(after)}}
    {% endif %}
    {% if is_not_blank(before) %}
    and id < {{text(before)}}
    {% endif %}
{% if order == "asc" %}
order by created_at ASC
{% else if order == "desc" %}
order by created_at DESC
{% endif %}
limit {{limit}};
)", context);
            ListVectorStoresResponse response;
            response.set_object("list");
            if (const auto n = list.size(); n>limit) {
                response.set_first_id(list.front().id());
                response.set_last_id(list[n-2].id());
                response.mutable_data()->Add(list.begin(), list.end()-1);
                response.set_has_more(true);
            } else {
                response.set_has_more(false);
                if (n>0) {
                    response.mutable_data()->Add(list.begin(), list.end());
                    response.set_first_id(list.front().id());
                    response.set_last_id(list.back().id());
                }
            }
            return response;
        }

        [[nodiscard]] size_t UpdateVectorStore(const ModifyVectorStoreRequest& update) const {
            SQLContext context;
            if (update.has_expires_after()) {
                assert_true(update.expires_after().anchor() == VectorStoreExpirationPolicy::last_active_at, "should have anchor set to last_active_at");
                assert_gt(update.expires_after().days(), 0, "should have expires_after.days set with a positive value");
            }
            ProtobufUtils::ConvertMessageToJsonObject(update, context);
            return data_template_->Execute(R"(
update instinct_vector_store
set
{% if exists("name") and is_not_blank(name) %}
    name = {{text(name)}} and
{% endif %}
{% if exists("expires_after") %}
    expires_after = {{stringify(expires_after)}} and
{% endif %}
{% if exists("metadata") %}
    metadata = {{stringify(metadata)}} and
{% endif %}
{% if exists("summary") and {{is_not_blank(summary)}} %}
    summary = {{text(summary)}} and
{% endif %}
    modified_at = now()
where id = {{text(vector_store_id)}};
)", context);
        }

        [[nodiscard]] size_t DeleteVectorStore(const DeleteVectorStoreRequest& req) const {
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(req, context);
            return data_template_->Execute(R"(
delete from instinct_vector_store
where id = {{text(vector_store_id)}};
)", context);
        }

        [[nodiscard]] std::optional<VectorStoreObject> GetVectorStore(const std::string& id) const {
            SQLContext context;
            context["id"] = id;
            return data_template_->SelectOne(R"(
select * from instinct_vector_store where id = {{text(id)}};
)", context);

        }

    };

    using VectorStoreDataMapperPtr = std::shared_ptr<VectorStoreDataMapper>;


}


#endif //VECTORSTOREDATAMAPPER_HPP
