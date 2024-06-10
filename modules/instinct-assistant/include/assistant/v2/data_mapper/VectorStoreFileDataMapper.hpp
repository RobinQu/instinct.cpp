//
// Created by RobinQu on 2024/5/27.
//

#ifndef VECTORSTOREFILEDATAMAPPER_HPP
#define VECTORSTOREFILEDATAMAPPER_HPP

#include "AssistantGlobals.hpp"
#include "database/BaseConnectionPool.hpp"
#include "database/IDataTemplate.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;

    class VectorStoreFileDataMapper {
        DataTemplatePtr<VectorStoreFileObject, std::string> data_template_;
    public:
        explicit VectorStoreFileDataMapper(DataTemplatePtr<VectorStoreFileObject, std::string> data_template)
            : data_template_(std::move(data_template)) {
        }

        [[nodiscard]] size_t InsertVectorStoreFile(const CreateVectorStoreFileRequest& create_vector_store_file_request) const {
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(create_vector_store_file_request, context);
            return data_template_->Execute(R"(
insert into instinct_vector_store_file(file_id, vector_store_id, status) values(
    {{text(file_id)}},
    {{text(vector_store_id)}},
    'in_progress'
);
)", context);
        }

        std::vector<std::string> InsertManyVectorStoreFiles(const std::string& vector_store_id, const RangeOf<std::string> auto & file_ids, const std::string& batch_id = "") {
            assert_non_empty_range(file_ids, "should have at least one file id");
            SQLContext context;
            context["vector_store_id"] = vector_store_id;
            context["file_ids"] = file_ids;
            context["file_batch_id"] = batch_id;
            return data_template_->InsertMany(R"(
insert into instinct_vector_store_file (file_id, vector_store_id, status, file_batch_id) values
## for file_id in file_ids
(
    {{text(file_id)}},
    {{text(vector_store_id)}},
    'in_progress',
    {{text(file_batch_id)}}
),
## endfor
;
)", context);
        }

        [[nodiscard]] ListVectorStoreFilesResponse ListVectorStoreFiles(const ListVectorStoreFilesRequest &req) const {
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(req, context, {.keep_default_values = true});
            // limit + 1 to check if there is more records that match the conditions
            auto limit = req.limit() <= 0 ? DEFAULT_LIST_LIMIT + 1 : req.limit() + 1;
            context["limit"] = limit;
            if (req.order() == unknown_list_request_order) {
                context["order"] = "desc";
            }
            if (req.filter() == VectorStoreFileStatus::unknown_vector_store_file_status) {
                context.erase("filter");
            }
            const auto list = data_template_->SelectMany(R"(
select * from instinct_vector_store_file
where
    vector_store_id = {{text(vector_store_id)}}
{% if exists("filter") and is_not_blank(filter) %}
    and filter = {{text(filter)}}
{% endif %}
{% if is_not_blank(after) %}
    and file_id > {{text(after)}}
{% endif %}
    {% if is_not_blank(before) %}
    and file_id < {{text(before)}}
{% endif %}
{% if order == "asc" %}
order by created_at ASC
{% else if order == "desc" %}
order by created_at DESC
{% endif %}
limit {{limit}};
)", context);
            ListVectorStoreFilesResponse response;
            response.set_object("list");
            if (const auto n = list.size(); n>limit) {
                response.set_first_id(list.front().file_id());
                response.set_last_id(list[n-2].file_id());
                response.mutable_data()->Add(list.begin(), list.end()-1);
                response.set_has_more(true);
            } else {
                response.set_has_more(false);
                if (n>0) {
                    response.mutable_data()->Add(list.begin(), list.end());
                    response.set_first_id(list.front().file_id());
                    response.set_last_id(list.back().file_id());
                }
            }
            return response;
        }


        std::vector<VectorStoreFileObject> ListVectorStoreFiles(const std::string& vector_store_id, const RangeOf<std::string> auto& file_ids) {
            SQLContext context;
            context["vector_store_id"] = vector_store_id;
            context["file_ids"] = file_ids;
            return data_template_->SelectMany(R"(
select * from instinct_vector_store_file
where vector_store_id = {{text(vector_store_id)}} and file_id in (
## for file_id in file_ids
    {{text(file_id)}},
## endfor
);
)", context);
        }

        std::vector<VectorStoreFileObject> ListVectorStoreFiles(const RangeOf<std::string> auto& vector_store_ids) {
            SQLContext context;
            context["vector_store_ids"] = vector_store_ids;
            return data_template_->SelectMany(R"(select * from instinct_vector_store_file
where vector_store_id in (
## for vs_id in vector_store_ids
    {{text(vs_id)}},
## endfor
);
)", context);
        }

        [[nodiscard]] ListFilesInVectorStoreBatchResponse ListVectorStoreFiles(const ListFilesInVectorStoreBatchRequest& req) const {
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(req, context, {.keep_default_values = true});
            auto limit = req.limit() <= 0 ? DEFAULT_LIST_LIMIT + 1 : req.limit() + 1;
            context["limit"] = limit;
            if (req.order() == unknown_list_request_order) {
                context["order"] = "desc";
            }
            const auto list = data_template_->SelectMany(R"(
select * from instinct_vector_store_file
where
    vector_store_id = {{text(vector_store_id)}}
    and file_batch_id = {{text(batch_id)}}
{% if is_not_blank(after) %}
    and file_id > {{text(after)}}
{% endif %}
    {% if is_not_blank(before) %}
    and file_id < {{text(before)}}
{% endif %}
{% if order == "asc" %}
order by created_at ASC
{% else if order == "desc" %}
order by created_at DESC
{% endif %}
limit {{limit}};
)", context);
            ListFilesInVectorStoreBatchResponse response;
            response.set_object("list");
            if (const auto n = list.size(); n>limit) {
                response.set_first_id(list.front().file_id());
                response.set_last_id(list[n-2].file_id());
                response.mutable_data()->Add(list.begin(), list.end()-1);
                response.set_has_more(true);
            } else {
                response.set_has_more(false);
                if (n>0) {
                    response.mutable_data()->Add(list.begin(), list.end());
                    response.set_first_id(list.front().file_id());
                    response.set_last_id(list.back().file_id());
                }
            }
            return response;
        }

        [[nodiscard]] VectorStoreObject_FileCounts CountVectorStoreFiles(const std::string& vector_store_id) const {
            SQLContext context;
            context["vector_store_id"] = vector_store_id;
            const auto aggregations = data_template_->Aggregate(R"(
select
    sum(case when status = 'in_progress' then 1 else 0 end) as in_progress,
    sum(case when status = 'completed' then 1 else 0 end) as completed,
    sum(case when status = 'failed' then 1 else 0 end) as failed,
    sum(case when status = 'cancelled' then 1 else 0 end) as cancelled,
    count(*) as total
from instinct_vector_store_file
where vector_store_id = {{text(vector_store_id)}};
)", context);
            VectorStoreObject_FileCounts file_counts;
            assert_gt(aggregations.rows_size(), 0, "should have at least one row");
            auto& row = aggregations.rows(0);
            file_counts.set_in_progress(static_cast<int32_t>(row.int64().contains("in_progress") ? row.int64().at("in_progress") : 0));
            file_counts.set_cancelled(static_cast<int32_t>(row.int64().contains("cancelled") ? row.int64().at("cancelled") : 0));
            file_counts.set_failed(static_cast<int32_t>(row.int64().contains("failed") ? row.int64().at("failed") : 0));
            file_counts.set_completed(static_cast<int32_t>(row.int64().contains("completed") ? row.int64().at("completed") : 0));
            file_counts.set_total(static_cast<int32_t>(row.int64().contains("total") ? row.int64().at("total") : 0));
            return file_counts;
        }

        [[nodiscard]] size_t DeleteVectorStoreFile(const std::string& vector_store_id, const std::string& vector_store_file_id) const {
            SQLContext context;
            context["vector_store_id"] = vector_store_id;
            context["vector_store_file_id"] = vector_store_file_id;
            return data_template_->Execute(R"(
delete from instinct_vector_store_file
where vector_store_id = {{text(vector_store_id)}} and file_id = {{text(vector_store_file_id)}};
)", context);
        }

        [[nodiscard]] size_t DeleteVectorStoreFiles(const std::string& vector_store_id) const {
            SQLContext context;
            context["vector_store_id"] = vector_store_id;
            return data_template_->Execute(R"(
delete from instinct_vector_store_file
where vector_store_id = {{text(vector_store_id)}};
)", context);
        }

        [[nodiscard]] std::optional<VectorStoreFileObject> GetVectorStoreFile(const std::string& vector_store_id, const std::string& vector_store_file_id) const {
            SQLContext context;
            context["id"] = vector_store_file_id;
            context["vector_store_id"] = vector_store_id;
            return data_template_->SelectOne(R"(
select * from instinct_vector_store_file
where file_id = {{text(id)}} and vector_store_id = {{text(vector_store_id)}};
)", context);
        }

        [[nodiscard]] size_t CancelVectorStoreFiles(const std::string& vector_store_id, const std::string& batch_id) const {
            assert_not_blank(batch_id, "should provide batch_id");
            assert_not_blank(vector_store_id, "should provide vector_store_id");
            // assert_non_empty_range(file_ids, "should have at least one file id");
            SQLContext context;
            // context["file_ids"] = file_ids;
            context["vector_store_id"] = vector_store_id;
            context["batch_id"] = batch_id;
            return data_template_->Execute(R"(
update instinct_vector_store_file
set
    status = 'cancelled',
    modified_at = now()
where
    vector_store_id = {{text(vector_store_id)}}
    and file_batch_id = {{text(batch_id)}}
    and status = 'in_progress';)", context);
        }

        [[nodiscard]] size_t UpdateVectorStoreFile(const ModifyVectorStoreFileRequest& update) const {
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(update, context);
            return data_template_->Execute(R"(
update instinct_vector_store_file
set
{% if exists("status") %}
    status = {{text(status)}},
{% endif %}
{% if exists("last_error") %}
    last_error = {{stringify(last_error)}},
{% endif %}
{% if exists("summary") and is_not_blank(summary) %}
    summary = {{text(summary)}},
{% endif %}
    modified_at = now()
where
    vector_store_id = {{text(vector_store_id)}}
    and file_id = {{text(file_id)}}
    and status = 'in_progress';
)", context);
        }
    };

    using VectorStoreFileDataMapperPtr = std::shared_ptr<VectorStoreFileDataMapper>;

}

#endif //VECTORSTOREFILEDATAMAPPER_HPP
