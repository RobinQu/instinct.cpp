//
// Created by RobinQu on 2024/4/25.
//

#ifndef ENTITYSQLUTILS_HPP
#define ENTITYSQLUTILS_HPP

#include <instinct/assistant_global.hpp>
#include <instinct/database/data_template.hpp>


namespace INSTINCT_ASSISTANT_NS {
    using namespace INSTINCT_DATA_NS;
    using namespace v2;

    namespace details {
        static void check_presence(const SQLContext& context, const std::vector<std::string>& names) {
            for (const auto& field: names) {
                assert_true(context.contains(field), fmt::format("should provide {} in context", field));
            }
        }
    }

    class EntitySQLUtils final {
    public:
        template<typename PrimaryKey = std::string>
        static PrimaryKey InsertOneAssistant(const DataTemplatePtr<AssistantObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            for (const auto& field: { "id", "model", "temperature", "top_p"}) {
                assert_true(context.contains(field), fmt::format("should provide {} in context", field));
            }
            return data_mapper->InsertOne(R"(
insert into instinct_assistant(
    id,
    name,
    model,
    description,
    instructions,
    temperature,
    top_p
{% if exists("tools") %}
    , tools
{% endif %}
{% if exists("tool_resources") %}
    , tool_resources
{% endif %}
    , response_format
{% if exists("metadata") %}
    metadata
{% endif %}
)  values(
    {{text(id)}},
    {{text(name)}},
    {{text(model)}},
    {{text(description)}},
    {{text(instructions)}},
    {{temperature}},
    {{top_p}}
{% if exists("tools") %}
    , {{stringify(tools)}}
{% endif %}
{% if exists("tool_resources") %}
    , {{stringify(tool_resources)}}
{% endif %}
    , {{text(response_format)}}
{% if exists("metadata") %}
    , {{stringify(metadata)}}
{% endif %}
);
)", context);
        }

        template<typename PrimaryKey = std::string>
        static std::vector<AssistantObject> GetManyAssistant(const DataTemplatePtr<AssistantObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->SelectMany(R"(
select * from instinct_assistant
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
        }

        template<typename PrimaryKey = std::string>
        static std::optional<AssistantObject> GetOneAssistant(const DataTemplatePtr<AssistantObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->SelectOne("select * from instinct_assistant where id = {{text(id)}}", context);
        }

        template<typename PrimaryKey = std::string>
        static size_t DeleteAssistant(const DataTemplatePtr<AssistantObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->Execute("delete from instinct_assistant where id = {{text(id)}}", context);
        }

        template<typename PrimaryKey = std::string>
        static size_t UpdateAssistant(const DataTemplatePtr<AssistantObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->Execute(R"(
update instinct_assistant
set
    {% if exists("model") and is_not_blank(model) %}
    model = {{text(model)}},
    {% endif %}
    {% if exists("description") and is_not_blank(description) %}
    description = {{text(description)}},
    {% endif %}
    {% if exists("instruction") and is_not_blank(instructions) %}
    instructions = {{text(instructions)}},
    {% endif %}
    {% if exists("metadata") %}
    metadata = {{stringify(metadata)}},
    {% endif %}
    {% if exists("temperature") %}
    temperature = {{temperature}},
    {% endif %}
    {% if exists("top_p") %}
    top_p = {{top_p}},
    {% endif %}
    {% if exists("response_format") and is_not_blank(response_format) %}
    response_format = {{text(response_format)}},
    {% endif %}
    {% if exists("tools") %}
    tools = {{stringify(tools)}},
    {% endif %}
    {% if exists("tool_resources") %}
    tool_resources = {{stringify(tool_resources)}},
    {% endif %}
    modified_at = now()
where id = {{text(assistant_id)}}
 )", context);

        }


        template<typename PrimaryKey = std::string>
        static PrimaryKey InsertOneThread(const DataTemplatePtr<ThreadObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->InsertOne(R"(
insert into instinct_thread(id
{% if exists("tool_resources") %}
    ,tool_resources
{% endif %}
{% if exists("metadata") %}
    ,metadata
{% endif %}
) values (
    {{text(id)}}
{% if exists("tool_resources") %}
    ,{{stringify(tool_resources)}}
{% endif %}
{% if exists("metadata") %}
    ,{{stringify(metadata)}}
{% endif %}
)
returning (id);
            )", context);
        }

        template<typename PrimaryKey = std::string>
        static size_t UpdateThread(const DataTemplatePtr<ThreadObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->Execute(R"(
update instinct_thread
set
    modified_at = now()
    {% if exists("tool_resources") %}
    ,tool_resources = {{stringify(tool_resources)}}
    {% endif %}
    {% if exists("metadata")  %}
    , metadata = {{stringify(metadata)}}
    {% endif %}
where id = {{text(thread_id)}};
            )", context);
        }

        template<typename PrimaryKey = std::string>
        static size_t DeleteThread(const DataTemplatePtr<ThreadObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->Execute("delete from instinct_thread where id = {{text(thread_id)}};", context);
        }


        template<typename PrimaryKey = std::string>
        static size_t UpdateMessage(const DataTemplatePtr<MessageObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->Execute(R"(
update instinct_thread_message
set
    modified_at = now()
    {% if exists("metadata") %}
    , metadata = {{stringify(metadata)}}
    {% endif %}
where id = {{text(message_id)}} and thread_id = {{text(thread_id)}};
)", context);
        }

        template<typename PrimaryKey = std::string>
        static std::optional<MessageObject> SelectOneMessages(const DataTemplatePtr<MessageObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->SelectOne("select * from instinct_thread_message where id = {{text(message_id)}} and thread_id={{text(thread_id)}};", context);
        }

        template<typename PrimaryKey = std::string>
        static std::vector<MessageObject> SelectManyMessages(const DataTemplatePtr<MessageObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->SelectMany(R"(
select * from instinct_thread_message
where
    thread_id = {{text(thread_id)}}
    {% if exists("run_id") and is_not_blank("run_id") %}
    and run_id = {{text(run_id)}}
    {% endif %}
    {% if exists("after") and is_not_blank("after") %}
    and id > {{text(after)}}
    {% endif %}
    {% if exists("before") and is_not_blank("before") %}
    and id < {{text(before)}}
    {% endif %}
{% if exists("order") %}
    {% if order == "asc" %}
    order by created_at asc
    {% else %}
    order by created_at desc
    {% endif %}
{% endif %}
limit {{limit}};
            )", context);
        }

        template<typename PrimaryKey = std::string>
        static std::vector<PrimaryKey> InsertManyMessages(const DataTemplatePtr<MessageObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            if (!context.contains("messages") || context["messages"].empty()) return {};
            for(const auto& msg_obj: context["messages"]) {
                assistant::details::check_presence(msg_obj, {"id", "thread_id", "content", "role", "status"});
                assert_not_blank(msg_obj["thread_id"], "should provide thread_id");
                assert_true(msg_obj.at("content").is_array(), "should provide content");
                for(const auto& content_item: msg_obj.at("content")) {
                    assert_true(content_item["type"] == "text" || content_item["type"] == "image_file", "content type for message should be text or image_file.");
                }
                assert_true((msg_obj.at("role").get<std::string>() == "user" || msg_obj.at("role").get<std::string>() == "assistant"), "should provide correct role for message");
            }
            return data_mapper->InsertMany(R"(
insert into instinct_thread_message(
    id,
    thread_id,
    status,
    incomplete_details,
    completed_at,
    incompleted_at,
    role,
    content,
    assistant_id,
    run_id,
    attachments,
    metadata
) values
## for msg in messages
(
    {{text(msg.id)}},
    {{text(msg.thread_id)}},
    {{text(msg.status)}},
{% if existsIn(msg, "incomplete_details") %}
    {{stringify(msg.incomplete_details)}},
{% else %}
    NULL,
{% endif %}
{% if existsIn(msg, "completed_at") and msg.completed_at %}
    {{timestamp(msg.completed_at)}},
{% else %}
    NULL,
{% endif %}
{% if existsIn(msg, "incompleted_at") and msg.incompleted_at %}
    {{timestamp(msg.incompleted_at)}},
{% else %}
    NULL,
{% endif %}
    {{text(msg.role)}},
    {{stringify(msg.content)}},
{% if existsIn(msg,"assistantId") and is_not_blank(msg.assistant_id) %}
    {{text(msg.assistant_id)}},
{% else %}
    NULL,
{% endif %}
{% if existsIn(msg,"run_id") and is_not_blank(msg.run_id) %}
    {{text(msg.run_id)}},
{% else %}
    NULL,
{% endif %}
{% if existsIn(msg, "attachments") %}
    {{stringify(msg.attachments)}},
{% else %}
    NULL,
{% endif %}
{% if existsIn(msg, "metadata") %}
    {{stringify(msg.metadata)}},
{% else %}
    NULL,
{% endif %}
),
## endfor
returning (id);
)", context);
        }


        template<typename PrimaryKey = std::string>
        static size_t DeleteManyMessages(const DataTemplatePtr<MessageObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->Execute(R"(
delete from instinct_thread_message where thread_id = {{text(thread_id)}};
            )", context);
        }

        template<typename PrimaryKey = std::string>
        static PrimaryKey InsertOneMessages(const DataTemplatePtr<MessageObject, PrimaryKey>& data_mapper, const SQLContext& msg_obj) {
            assistant::details::check_presence(msg_obj, {"id", "thread_id", "content", "role", "status"});
            assert_true(msg_obj.at("content").is_array(), "should provide content");
            for(const auto& content_item: msg_obj.at("content")) {
                assert_true(content_item["type"] == "text" || content_item["type"] == "image_file", "content type for message should be text or image_file.");
            }

            assert_true((msg_obj.at("role").get<std::string>() == "user" || msg_obj.at("role").get<std::string>() == "assistant"), "should provide correct role for message");

            return data_mapper->InsertOne(R"(
insert into instinct_thread_message(id, thread_id, status, incomplete_details, completed_at, incompleted_at, role, content, assistant_id, run_id, attachments, metadata) values
(
    {{text(id)}},
    {{text(thread_id)}},
    {{text(status)}},
{% if exists("incomplete_details") %}
    {{stringify(incomplete_details)}},
{% else %}
    NULL,
{% endif %}
{% if exists("completed_at") and completed_at %}
    {{timestamp(completed_at)}},
{% else %}
    NULL,
{% endif %}
{% if exists("incompleted_at") and incompleted_at %}
    {{timestamp(incompleted_at)}},
{% else %}
    NULL,
{% endif %}
    {{text(role)}},
{% if exists("content") %}
    {{stringify(content)}},
{% else %}
    NULL,
{% endif %}
{% if exists("assistantId") and is_not_blank(assistant_id) %}
    {{text(assistant_id)}},
{% else %}
    NULL,
{% endif %}
{% if exists("run_id") and is_not_blank(run_id) %}
    {{text(run_id)}},
{% else %}
    NULL,
{% endif %}
{% if exists("attachments") %}
    {{stringify(attachments)}},
{% else %}
    NULL,
{% endif %}
{% if exists("metadata") %}
    {{stringify(metadata)}},
{% else %}
    NULL,
{% endif %}
)
returning (id);
)", msg_obj);
        }

        template<typename PrimaryKey = std::string>
        static PrimaryKey InsertOneFile(const DataTemplatePtr<FileObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            assistant::details::check_presence(context, {"id", "filename", "bytes", "purpose"});
            return data_mapper->InsertOne(R"(
insert into instinct_file(id, filename, bytes, purpose) values(
    {{text(id)}},
    {{text(filename)}},
    {{bytes}},
    {{text(purpose)}}
);
            )", context);
        }

        template<typename PrimaryKey = std::string>
        static std::vector<FileObject> SelectManyFiles(const DataTemplatePtr<FileObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->SelectMany(R"(
select * from instinct_file
where 1=1
{% if exists("purpose") and is_not_blank("purpose") %}
and purpose = {{text(purpose)}}
{% endif %}
order by created_at desc
            )", context);
        }

        template<typename PrimaryKey = std::string>
        static std::optional<FileObject> SelectOneFile(const DataTemplatePtr<FileObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->SelectOne(R"(
select * from instinct_file where id = {{text(file_id)}};
            )", context);
        }

        template<typename PrimaryKey = std::string>
        static size_t DeleteFile(const DataTemplatePtr<FileObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->Execute("delete from instinct_file where id = {{text(file_id)}}", context);
        }

        template<typename PrimaryKey = std::string>
        static PrimaryKey InsertOneRun(const DataTemplatePtr<RunObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            assistant::details::check_presence(context, {"thread_id", "assistant_id"});
            return data_mapper->InsertOne(R"(
insert into instinct_thread_run(
    id, thread_id, assistant_id, status, truncation_strategy, response_format
    {% if exists("model") and is_not_blank("model") %}
    , model
    {% endif %}
    {% if exists("instructions") and is_not_blank("instructions") %}
    , instructions
    {% endif %}
    {% if exists("tools") %}
    , tools
    {% endif %}
    {% if exists("tool_resources") %}
    , tool_resources
    {% endif %}
    {% if exists("metadata") %}
    , metadata
    {% endif %}
    {% if exists("temperature") %}
    , temperature
    {% endif %}
    {% if exists("top_p") %}
    , top_p
    {% endif %}
    {% if exists("max_prompt_tokens") and max_prompt_tokens %}
    , max_prompt_tokens
    {% endif %}
    {% if exists("max_completion_tokens") and max_completion_tokens %}
    , max_completion_tokens
    {% endif %}
    {% if exists("tool_choice") and is_not_blank("tool_choice") %}
    , tool_choice
    {% endif %}
) values(
    {{text(id)}}
    , {{text(thread_id)}}
    , {{text(assistant_id)}}
    , {{text(status)}}
    , {{stringify(truncation_strategy)}}
    , {{text(response_format)}}
    {% if exists("model") and is_not_blank("model") %}
    , {{text(model)}}
    {% endif %}
    {% if exists("instructions") and is_not_blank("instructions") %}
    , {{text(instructions)}}
    {% endif %}
    {% if exists("tools") %}
    , {{stringify(tools)}}
    {% endif %}
    {% if exists("tool_resources") %}
    , {{stringify(tool_resources)}}
    {% endif %}
    {% if exists("metadata") %}
    , {{stringify(metadata)}}
    {% endif %}
    {% if exists("temperature") %}
    , {{temperature}}
    {% endif %}
    {% if exists("top_p") %}
    , {{top_p}}
    {% endif %}
    {% if exists("max_prompt_tokens") and max_prompt_tokens %}
    , {{max_prompt_tokens}}
    {% endif %}
    {% if exists("max_completion_tokens") and max_completion_tokens %}
    , {{max_completion_tokens}}
    {% endif %}
    {% if exists("tool_choice") and is_not_blank("tool_choice") %}
    , {{text(tool_choice)}}
    {% endif %}
) returning (id);
            )", context);
        }

        template<typename PrimaryKey = std::string>
        static size_t UpdateRun(const DataTemplatePtr<RunObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->Execute(R"(
update instinct_thread_run
set
    modified_at = now()
    {% if exists("status") and is_not_blank(status) %}
    , status = {{text(status)}}
    {% endif %}
    {% if exists("metadata") %}
    , metadata = {{stringify(metadata)}}
    {% endif %}
    {% if exists("required_action") %}
    , required_action = {{stringify(required_action)}}
    {% endif %}
    {% if exists("started_at") and started_at > 0 %}
    , started_at = {{timestamp(started_at)}}
    {% endif %}
    {% if exists("expires_at") and expires_at > 0 %}
    , expires_at = {{timestamp(expires_at)}}
    {% endif %}
    {% if exists("cancelled_at") and cancelled_at > 0 %}
    , cancelled_at = {{timestamp(cancelled_at)}}
    {% endif %}
    {% if exists("failed_at") and failed_at > 0 %}
    , failed_at = {{timestamp(failed_at)}}
    {% endif %}
    {% if exists("completed_at") and completed_at > 0 %}
    , completed_at = {{timestamp(completed_at)}}
    {% endif %}
where thread_id = {{text(thread_id)}} and id = {{text(run_id)}};
            )", context);
        }


        template<typename PrimaryKey = std::string>
        static std::vector<RunObject> SelectManyRuns(const DataTemplatePtr<RunObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            assert_true(context.contains("thread_id"), "should provide thread_id");
            return data_mapper->SelectMany(R"(
select * from instinct_thread_run
where
    thread_id = {{text(thread_id)}}
    {% if exists("after") and is_not_blank(after) %}
    and id > after
    {% endif %}
    {% if exists("before") and is_not_blank(before) %}
    and id < before
    {% endif %}
    {% if exists("possible_statuses")  %}
    and status in (
        {% for status in possible_statuses %}
        {{text(status)}},
        {% endfor %}
    )
    {% endif %}
{% if order == "asc" %}
order by created_at asc
{% endif %}
{% if order == "desc" %}
order by created_at desc
{% endif %}
limit {{limit}};
            )", context);
        }

        template<typename PrimaryKey = std::string>
        static std::optional<RunObject> SelectOneRun(const DataTemplatePtr<RunObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            assert_true(context.contains("run_id"), "should provide run id");
            assert_true(context.contains("thread_id"), "should provide thread id");
            return data_mapper->SelectOne(R"(
select * from instinct_thread_run
where id = {{text(run_id)}} and thread_id={{text(thread_id)}}
limit 1;
            )", context);
        }


        template<typename PrimaryKey = std::string>
        static size_t DeleteManyRuns(const DataTemplatePtr<RunObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->Execute(R"(
delete from instinct_thread_run where thread_id = {{text(thread_id)}};
            )", context);
        }

        template<typename PrimaryKey = std::string>
        static PrimaryKey InsertOneRunStep(const DataTemplatePtr<RunStepObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->InsertOne(R"(
insert into instinct_thread_run_step(id, thread_id, run_id, type, status, step_details)
values (
    {{text(id)}},
    {{text(thread_id)}},
    {{text(run_id)}},
    {{text(type)}},
    {{text(status)}},
{% if exists("step_details") %}
    {{stringify(step_details)}}
{% else %}
    NULL
{% endif %}
)
returning (id);
            )", context);
        }


        template<typename PrimaryKey = std::string>
        static size_t DeleteManyRunSteps(const DataTemplatePtr<RunStepObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->Execute(R"(
delete from instinct_thread_run_step where thread_id = {{text(thread_id)}};
            )", context);
        }

        template<typename PrimaryKey = std::string>
        static size_t UpdateRunStep(const DataTemplatePtr<RunStepObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->Execute(R"(
update instinct_thread_run_step
set
    modified_at = now()
    {% if exists("status") and is_not_blank(status) %}
    , status = {{text(status)}}
    {% endif %}

    {% if exists("step_details") %}
    , step_details = {{stringify(step_details)}}
    {% endif %}

    {% if exists("last_error") %}
    , last_error = {{stringify(last_error)}}
    {% endif %}

    {% if exists("expired_at") and expired_at > 0 %}
    , expired_at = {{timestamp(expired_at)}}
    {% endif %}

    {% if exists("cancelled_at") and cancelled_at > 0 %}
    , cancelled_at = {{timestamp(cancelled_at)}}
    {% endif %}

    {% if exists("failed_at") and failed_at > 0 %}
    , failed_at = {{timestamp(failed_at)}}
    {% endif %}

    {% if exists("completed_at") and completed_at > 0 %}
    , completed_at = {{timestamp(completed_at)}}
    {% endif %}
where
    id={{text(step_id)}}
    and thread_id={{text(thread_id)}}
    and run_id = {{text(run_id)}};
            )", context);
        }

        template<typename PrimaryKey = std::string>
        static std::optional<RunStepObject> GetRunStep(const DataTemplatePtr<RunStepObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->SelectOne("select * from instinct_thread_run_step where id={{text(step_id)}} and thread_id={{text(thread_id)}} and run_id = {{text(run_id)}} limit 1;", context);
        }

        template<typename PrimaryKey = std::string>
        static std::vector<RunStepObject> SelectManyRunSteps(const DataTemplatePtr<RunStepObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->SelectMany(R"(
select * from instinct_thread_run_step
where
    thread_id = {{text(thread_id)}}
    and run_id = {{text(run_id)}}
    {% if exists("after") and is_not_blank(after) %}
    and id > {{text(after)}}
    {% endif %}
    {% if exists("before") and is_not_blank(before) %}
    and id < {{text(before)}}
    {% endif %}
{% if order == "asc" %}
order by created_at asc
{% endif %}
{% if order == "desc" %}
order by created_at desc
{% endif %}
limit {{limit}};
            )", context);
        }

    };


}


#endif //ENTITYSQLUTILS_HPP
