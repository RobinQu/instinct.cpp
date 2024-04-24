//
// Created by RobinQu on 2024/4/22.
//

#ifndef ASSISTANTSERVICEIMPL_HPP
#define ASSISTANTSERVICEIMPL_HPP

#include "AssistantGlobals.hpp"
#include "assistant/v2/service/IAssistantService.hpp"
#include "database/IDataMapper.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;

    class AssistantServiceImpl final: public IAssistantService {
        DataMapperPtr<AssistantObject, std::string> data_mapper_;
    public:
        explicit AssistantServiceImpl(const DataMapperPtr<AssistantObject, std::string> &data_mapper)
            : data_mapper_(data_mapper) {
        }

        ListAssistantsResponse ListAssistants(const ListAssistantsRequest &list_request) override {
            // limit + 1 to check if there is more records that match the conditions
            auto limit = list_request.limit() == 0 ? DEFAULT_LIST_LIMIT + 1 : list_request.limit() + 1;
            auto assistants = data_mapper_->SelectMany(R"(
select from instinct_assistant
where
    {% if after %}
    id > {{after}} and
    {% endif %}
    {% if before &}
    id < {{before}}
    {% endif %}
{% if order == "asc" %}
order by created_at ASC
{% else if order == "desc" %}
order by created_at DESC
{% endif %}
limit {% limit %};
)", {
                {"after", list_request.after()},
                {"before", list_request.before()},
                {"limit", limit},
                {"order", to_string(list_request.order()) }
            });

            ListAssistantsResponse response;
            if (assistants.size() == limit) { // no more
                response.mutable_data()->Add(assistants.begin(), assistants.end());
                response.set_first_id(assistants.front().id());
                response.set_last_id(assistants.back().id());
                response.set_has_more(false);
            } else { // at least have one more
                response.mutable_data()->Add(assistants.begin(), assistants.end()-1);
                response.set_first_id(assistants.front().id());
                if (assistants.size() > 2) {
                    response.set_last_id(assistants.end()[-2].id());
                } else {
                    response.set_last_id(assistants.front().id());
                }
                response.set_has_more(true);
            }
            return response;
        }

        std::optional<AssistantObject> CreateAssistant(const AssistantObject &create_request) override {
            assert_not_blank(create_request.model(), "should provide model name");
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(create_request, context, {.keep_default_values = true});

            if (!create_request.has_top_p()) {
                context["top_p"] = 1;
            }
            if (!create_request.has_temperature()) {
                context["temperature"] = 1;
            }

            auto id = details::generate_next_object_id("assistant");
            context["id"] = id;
            data_mapper_->Execute(R"(
insert into instinct_assistant(
    id,
    name,
    model,
    description,
    instructions,
    temperature,
    top_p,
{% if exists("tools") %}
    tools,
{% endif %}
{% if exists("tool_resourecs") %}
    tool_resourecs,
{% endif %}
{% if exists("metadata") %}
    metadata,
{% endif %}
    created_at,
    modified_at
)  values(
    {{text(id)}},
    {{text(model)}},
    {{text(description)}},
    {{text(instructions)}},
    {{temperature}},
    {{top_p}},
{% if exists("tools") %}
    {{stringify(tools)}},
{% endif %}
{% if exists("tool_resourecs") %}
    {{stringify(tool_resources)}},
{% endif %}
    {{response_format}},
{% if exists("metadata") %}
    {{stringify(metadata)}},
{% endif %}
    now(),
    now()
);
)", context);

            GetAssistantRequest get_assistant_request;
            get_assistant_request.set_assistant_id(id);
            return this->RetrieveAssistant(get_assistant_request);
        }

        std::optional<AssistantObject> RetrieveAssistant(const GetAssistantRequest &get_request) override {
            return data_mapper_->SelectOne("select * from instinct_assistant where id = {{id}}", {{"id", get_request.assistant_id()}});
        }

        DeleteAssistantResponse DeleteAssistant(const DeleteAssistantRequest &delete_request) override {
            const auto count = data_mapper_->Execute("delete from instinct_assistant where id = {{id}}", {{"id", delete_request.assistant_id()}});

            DeleteAssistantResponse response;
            response.set_id(delete_request.assistant_id());
            response.set_deleted(count == 1);
            response.set_object("assistant.deleted");
            return response;
        }

        std::optional<AssistantObject> ModifyAssistant(const ModifyAssistantRequest &modify_assistant_request) override {
            assert_not_blank(modify_assistant_request.assistant_id(), "assistant id should be given");
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(modify_assistant_request, context);
            data_mapper_->Execute(R"("update instinct_assistant set
{% if is_not_blank(model) %}
model = {{model}},
{% endif %}
{% if is_not_blank(description) %}
description = {{description}},
{% endif %}
{% if is_not_blank(instructions) %}
instructions = {{instructions}},
{% endif %}
{% if is_not_blank(metadata) %}
metadata = {{metadata}},
{% endif %}
{% if is_not_blank(temperature) %}
temperature = {{temperature}},
{% endif %}
{% if is_not_blank(top_p) %}
top_p = {{top_p}},
{% endif %}
{% if is_not_blank(response_format) %}
response_format = {{response_format}},
{% endif %}
{% if exists("tools") %}
tools = {{stringify(tools)}},
{% endif %}
{% if exists("tool_resources") %}
tool_resources = {{stringify(tool_resources)}},
{% endif %}
{% if exists("metadata") %}
metadata = {{stringify(metadata)}},
{% endif %}
modified_at = now()
where id = {{assistant_id}}
 )", context);

            GetAssistantRequest get_assistant_request;
            get_assistant_request.set_assistant_id(modify_assistant_request.assistant_id());
            return this->RetrieveAssistant(get_assistant_request);
        }

    };
}


#endif //ASSISTANTSERVICEIMPL_HPP
