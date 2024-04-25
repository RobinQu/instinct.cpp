//
// Created by RobinQu on 2024/4/24.
//

#ifndef THREADSERVICEIMPL_HPP
#define THREADSERVICEIMPL_HPP
#include "AssistantTestGlobals.hpp"
#include "assistant/v2/service/IMessageService.hpp"
#include "assistant/v2/service/IThreadService.hpp"
#include "assistant/v2/tool/EntitySQLUtils.hpp"


namespace INSTINCT_ASSISTANT_NS {
    class ThreadServiceImpl final: public IThreadService {
        DataMapperPtr<ThreadObject, std::string> thread_data_mapper_;
        DataMapperPtr<MessageObject, std::string> message_data_mapper_;
    public:
        std::optional<ThreadObject> CreateThread(const ThreadObject &create_request) override {
            // TODO with transaction
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(create_request, context, {.keep_default_values = true});

            // generate thread id
            auto thread_id = details::generate_next_object_id("thread");
            context["id"] = thread_id;

            // generate mesage id
            for(auto& msg_obj: context["messages"]) {
                msg_obj["id"] = details::generate_next_object_id("message");
                msg_obj["thread_id"] = thread_id;
            }

            // create mesages
            EntitySQLUtils::InsertManyMessages(message_data_mapper_, context);

            // create thread
            EntitySQLUtils::InsertOneThread(thread_data_mapper_, context);

            GetThreadRequest get_thread_request;
            get_thread_request.set_thread_id(thread_id);
            return RetrieveThread(get_thread_request);
        }

        std::optional<ThreadObject> RetrieveThread(const GetThreadRequest &retrieve_request) override {
            assert_not_blank(retrieve_request.thread_id());
            return thread_data_mapper_->SelectOne("select * from instinct_thread where id = {{text(id)}};", {
                {"id", retrieve_request.thread_id()}
            });
        }

        std::optional<ThreadObject> ModifyThread(const ModifyThreadRequest &modify_request) override {
            assert_not_blank(modify_request.thread_id(), "should provide thread id");
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(modify_request, context);
            EntitySQLUtils::UpdateThread(thread_data_mapper_, context);
            GetThreadRequest get_thread_request;
            get_thread_request.set_thread_id(modify_request.thread_id());
            return RetrieveThread(get_thread_request);
        }

        DeleteThreadResponse DeleteThread(const DeleteThreadResponse &delete_request) override {
            assert_not_blank(delete_request.id(), "should provide thread id for  deletion");
            const auto count = EntitySQLUtils::DeleteThread(thread_data_mapper_, {{"id", delete_request.id()}});
            DeleteThreadResponse delete_thread_response;
            delete_thread_response.set_deleted(count == 1);
            delete_thread_response.set_object("thread.deleted");
            delete_thread_response.set_id(delete_request.id());
            return delete_thread_response;
        }

    };
}

#endif //THREADSERVICEIMPL_HPP
