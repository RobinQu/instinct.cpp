//
// Created by RobinQu on 2024/4/24.
//

#ifndef THREADSERVICEIMPL_HPP
#define THREADSERVICEIMPL_HPP
#include "assistant/v2/service/IThreadService.hpp"
#include "assistant/v2/tool/EntitySQLUtils.hpp"


namespace INSTINCT_ASSISTANT_NS::v2 {
    class ThreadServiceImpl final: public IThreadService {
        DataMapperPtr<ThreadObject, std::string> thread_data_mapper_;
        DataMapperPtr<MessageObject, std::string> message_data_mapper_;
        DataMapperPtr<RunObject, std::string> run_data_mapper_;
        DataMapperPtr<RunStepObject, std::string> run_step_data_mapper_;
    public:
        ThreadServiceImpl(
            const DataMapperPtr<ThreadObject, std::string> &thread_data_mapper,
            const DataMapperPtr<MessageObject, std::string> &message_data_mapper,
            const DataMapperPtr<RunObject, std::string>& run_data_mapper,
            const DataMapperPtr<RunStepObject, std::string>& run_step_data_mapper
            )
            : thread_data_mapper_(thread_data_mapper),
              message_data_mapper_(message_data_mapper),
              run_data_mapper_(run_data_mapper),
              run_step_data_mapper_(run_step_data_mapper) {
        }

        std::optional<ThreadObject> CreateThread(const ThreadObject &create_request) override {
            // TODO with transaction
            // generate thread id
            auto thread_id = details::generate_next_object_id("thread");

            if(create_request.messages_size()) {
                const auto new_messages_view = create_request.messages() | std::views::transform([&](const CreateMessageRequest& request) {
                    MessageObject message_object;
                    message_object.set_id(details::generate_next_object_id("msg"));
                    message_object.set_role(request.role());
                    auto* content = message_object.add_content();
                    content->set_type(MessageObject_MessageContentType_text);
                    content->mutable_text()->set_value(request.content());
                    message_object.set_status(MessageObject_MessageStatus_completed);
                    message_object.set_thread_id(thread_id);
                    message_object.set_completed_at(ChronoUtils::GetCurrentEpochMicroSeconds());
                    return message_object;
                });

                SQLContext create_messages_context;
                for(const auto& msg: new_messages_view) {
                    SQLContext obj;
                    ProtobufUtils::ConvertMessageToJsonObject(msg, obj);
                    create_messages_context["messages"].push_back(obj);
                }
                // create mesages
                EntitySQLUtils::InsertManyMessages(message_data_mapper_, create_messages_context);
            }

            // create thread
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(create_request, context, {.keep_default_values = true});
            context["id"] = thread_id;
            EntitySQLUtils::InsertOneThread(thread_data_mapper_, context);

            // return thread
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

        DeleteThreadResponse DeleteThread(const DeleteThreadRequest &delete_request) override {
            // TODO needs transaction
            assert_not_blank(delete_request.thread_id(), "should provide thread id for  deletion");

            // check if there are any active run objects
            SQLContext check_runs_context;
            check_runs_context["thread_id"] = delete_request.thread_id();
            check_runs_context["possible_statuses"] = std::vector<std::string> { "in_progress", "cancelling" };
            check_runs_context["order"] = "desc";
            check_runs_context["limit"] = 1;
            const auto transition_runs = EntitySQLUtils::SelectManyRuns(run_data_mapper_, check_runs_context);
            assert_true(
                transition_runs.empty(),
                fmt::format(
                    "Some run objects in this thread are in active state which prevents delete of this thread. thread_id={}, run_ids={}",
                    delete_request.thread_id(),
                    StringUtils::JoinWith(
                        transition_runs | std::views::transform([](const RunObject& run) { return run.id();}),
                        ","
                    )
                )
            );

            // delete run objects
            SQLContext delete_runs_context;
            delete_runs_context["thread_id"] = delete_request.thread_id();
            EntitySQLUtils::DeleteManyRuns(run_data_mapper_, delete_runs_context);

            // delete run steps
            SQLContext delete_run_steps_context;
            delete_run_steps_context["thread_id"] = delete_request.thread_id();
            EntitySQLUtils::DeleteManyRunSteps(run_step_data_mapper_, delete_run_steps_context);

            // delete messages
            SQLContext delete_messages_context;
            delete_messages_context["thread_id"] = delete_request.thread_id();
            EntitySQLUtils::DeleteManyMessages(message_data_mapper_, delete_messages_context);

            // delete thread itself
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(delete_request, context);
            const auto count = EntitySQLUtils::DeleteThread(thread_data_mapper_, context);
            DeleteThreadResponse delete_thread_response;
            delete_thread_response.set_deleted(count == 1);
            delete_thread_response.set_object("thread.deleted");
            delete_thread_response.set_id(delete_request.thread_id());
            return delete_thread_response;
        }

    };
}

#endif //THREADSERVICEIMPL_HPP
