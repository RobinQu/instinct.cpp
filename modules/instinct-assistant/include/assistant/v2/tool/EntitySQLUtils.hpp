//
// Created by RobinQu on 2024/4/25.
//

#ifndef ENTITYSQLUTILS_HPP
#define ENTITYSQLUTILS_HPP

#include "AssistantGlobals.hpp"
#include "database/IDataMapper.hpp"


namespace INSTINCT_ASSISTANT_NS {
    using namespace INSTINCT_DATA_NS;
    using namespace v2;
    class EntitySQLUtils final {

    public:
        template<typename PrimaryKey = std::string>
        static PrimaryKey InsertOneAssistant(const DataMapperPtr<AssistantObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->InsertOne(R"(
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
    response_format,
{% if exists("metadata") %}
    metadata,
{% endif %}
    created_at,
    modified_at
)  values(
    {{text(id)}},
    {{text(name)}},
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
    {{text(response_format)}},
{% if exists("metadata") %}
    {{stringify(metadata)}},
{% endif %}
    now(),
    now()
);
)", context);
        }

        template<typename PrimaryKey = std::string>
        static std::vector<AssistantObject> GetManyAssistant(const DataMapperPtr<AssistantObject, PrimaryKey>& data_mapper, const SQLContext& context) {
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
        static std::optional<AssistantObject> GetOneAssistant(const DataMapperPtr<AssistantObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->SelectOne("select * from instinct_assistant where id = {{text(id)}}", context);
        }

        template<typename PrimaryKey = std::string>
        static size_t DeleteAssistant(const DataMapperPtr<AssistantObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->Execute("delete from instinct_assistant where id = {{text(id)}}", context);
        }

        template<typename PrimaryKey = std::string>
        static size_t UpdateAssistant(const DataMapperPtr<AssistantObject, PrimaryKey>& data_mapper, const SQLContext& context) {
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
        static PrimaryKey InsertOneThread(const DataMapperPtr<ThreadObject, PrimaryKey>& data_mapper, const SQLContext& context) {
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
        static size_t UpdateThread(const DataMapperPtr<ThreadObject, PrimaryKey>& data_mapper, const SQLContext& context) {
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
        static size_t DeleteThread(const DataMapperPtr<ThreadObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->Execute("delete from instinct_thread where id = {{text(thread_id)}};", context);
        }


        template<typename PrimaryKey = std::string>
        static size_t UpdateMessage(const DataMapperPtr<MessageObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->Execute(R"(
update instinct_thread_message
set
    modified_at = now()
    {% if exists("metadata") %}
    , metadata = {{stringify(metadata)}}
    {% endif %}
where message_id = {{text(message_id)}} and thread_id = {{text(thread_id)}};
)", context);
        }

        template<typename PrimaryKey = std::string>
        static std::optional<MessageObject> SelectOneMessages(const DataMapperPtr<MessageObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->SelectOne("select * from instinct_message where id = {{text(id)}} and thread_id={{text(thread_id)}};", context);
        }

        template<typename PrimaryKey = std::string>
        static std::vector<MessageObject> SelectManyMessages(const DataMapperPtr<MessageObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->SelectMany(R"(
select * from instinct_thread_message
where
thread_id = {{text(thread_id)}}
{% if exists("run_id") and is_not_blank("run_id") %}
, run_id = {{text(run_id)}}
{% endif %}
{% if exists("after") and is_not_blank("after") %}
, after > {{text(after)}}
{% endif %}
{% if exists("before") and is_not_blank("before") %}
, before < {{text(before)}}
{% endif %}
{% if exiists("order") %}
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
        static std::vector<PrimaryKey> InsertManyMessages(const DataMapperPtr<MessageObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            if (!context.contains("messages") || context["messages"].empty()) return {};
            for(const auto& msg_obj: context["messages"]) {
                assert_true(msg_obj.contains("content") && msg_obj.at("content").is_object(), "should provide content");
                assert_true(msg_obj.contains("role") && (msg_obj.at("role").get<std::string>() == "user" || msg_obj.at("role").get<std::string>() == "assistant"), "should provide correct role for message");
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
    {{text(msg.assistant_id)}},
    {{text(msg.run_id)}},
{% if existsIn(msg, "attachments") %}
    {{stringify(msg.attachments)}},
{% else %}
    NULL,
{% endif %}
{% if existsIn(msg, "metadata") %}
    {{stringify(msg.metadata)}}
{% else %}
    NULL,
{% endif %}
),
## endfor
returning (id);
)", context);
        }

        template<typename PrimaryKey = std::string>
        static PrimaryKey InsertOneMessages(const DataMapperPtr<MessageObject, PrimaryKey>& data_mapper, const SQLContext& msg_obj) {
            assert_true(msg_obj.contains("content") && msg_obj.at("content").is_object(), "should provide content");
            assert_true(msg_obj.contains("role") && (msg_obj.at("role").get<std::string>() == "human" || msg_obj.at("role").get<std::string>() == "assistant"), "should provide correct role for message");

            return data_mapper->InsertMany(R"(
insert into instinct_thread_message(id, thread_id, status, incomplete_details, completed_at, incompleted_at, role, content, assistant_id, run_id, attachments, metadata) values
(
    {{text(id)}},
    {{text(status)}},
{% if exists("incomplete_details") %}
    {{stringify(incomplete_details)}},
{% else %}
    NULL,
{% endif %}
{% if exists("completed_at") and msg.completed_at %}
    {{timestamp(completed_at)}},
{% else %}
    NULL,
{% endif %}
{% if exists("incompleted_at") and msg.incompleted_at %}
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
    {{text(assistant_id)}},
    {{text(run_id)}},
{% exists("attachments") %}
    {{stringify(attachments)}},
{% else %}
    NULL,
{% endif %}
{% exists("metadata") %}
    {{stringify(metadata)}}
{% else %}
    NULL
{% endif %}
)
returning (id);
)", msg_obj);
        }

        template<typename PrimaryKey = std::string>
        static PrimaryKey InsertOneFile(const DataMapperPtr<FileObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->InsertOne(R"(
insert into instinct_file(id, filename, bytes, purpose) values(
    {{text(id)}},
    {{text(filename)}},
    {{bytes}}
    {{text(purpose)}}
);
            )", context);
        }

        template<typename PrimaryKey = std::string>
        static std::vector<FileObject> SelectManyFiles(const DataMapperPtr<FileObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->SelectMany(R"(
select * from instinct_file
where 1=1
{% if exists("purpose") and is_not_blank("purpose") %}
 purpose = {{text(purpose)}}
{% endif %}
            )", context);
        }

        template<typename PrimaryKey = std::string>
        static std::optional<FileObject> SelectOneFile(const DataMapperPtr<FileObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->SelectOne(R"(
select * from instinct_file where id = {{text(file_id)}};
            )");

        }

        template<typename PrimaryKey = std::string>
        static size_t DeleteFile(const DataMapperPtr<FileObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->Execute("delete instinct_file where id = {{text(file_id)}}", context);
        }



    };







}


#endif //ENTITYSQLUTILS_HPP
