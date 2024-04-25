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
        static PrimaryKey InsertOneThread(const DataMapperPtr<ThreadObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->InsertMany(R"(
insert into instinct_thread(id, created_at, modified_at, tool_resources, metadata) values
(
    {{text(id)}},
    {{created_at}},
    {{modified_at}},
    {{stringify(tool_resources)}},
    {{stringify(metadata)}}
),
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
    {% if exists("metadata") %}
    , metadata = {{stringify(metadata)}}
    {% endif %}
where thread_id = {{text(thread_id)}};
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
            return data_mapper->InsertMany(R"(
insert into instinct_thread_message(id, thread_id, status, incomplete_details, completed_at, incompleted_at, role, content, assistant_id, run_id, attachments, metadata) values
## for msg in messages
(
    {{text(msg.id)}},
    {{text(msg.thread_id)}},
    {{text(msg.status)}},
    {{text(msg.incomplete_details)}},
    {{msg.completed_at}},
    {{msg.incompleted_at}},
    {{text(msg.role)}},
    {{stringify(msg.content)}},
    {{text(msg.assistant_id)}},
    {{text(msg.run_id)}}
    {{stringify(msg.attachments)}},
    {{stringify(msg.metadata)}}
),
## endfor
returning (id);
)", context);
        }

        template<typename PrimaryKey = std::string>
        static PrimaryKey InsertOneMessages(const DataMapperPtr<MessageObject, PrimaryKey>& data_mapper, const SQLContext& context) {
            return data_mapper->InsertMany(R"(
insert into instinct_thread_message(id, thread_id, status, incomplete_details, completed_at, incompleted_at, role, content, assistant_id, run_id, attachments, metadata) values
(
    {{text(id)}},
    {{text(status)}},
    {{text(incomplete_details)}},
    {{completed_at}},
    {{incompleted_at}},
    {{text(role)}},
    {{stringify(content)}},
    {{text(assistant_id)}},
    {{text(run_id)}}
    {{stringify(attachments)}},
    {{stringify(metadata)}}
)
returning (id);
)", context);
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
