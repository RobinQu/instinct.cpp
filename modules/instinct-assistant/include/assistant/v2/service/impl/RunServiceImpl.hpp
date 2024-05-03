//
// Created by RobinQu on 2024/4/26.
//

#ifndef RUNSERVICEIMPL_HPP
#define RUNSERVICEIMPL_HPP

#include "../IRunService.hpp"
#include "assistant/v2/service/IMessageService.hpp"
#include "assistant/v2/task_handler/RunObjectTaskHandler.hpp"
#include "assistant/v2/tool/EntitySQLUtils.hpp"
#include "database/IDataMapper.hpp"
#include "task_scheduler/ThreadPoolTaskScheduler.hpp"
#include "tool/FunctionToolUtils.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;

    class RunServiceImpl final: public IRunService {
        DataMapperPtr<ThreadObject, std::string> thread_data_mapper_;
        DataMapperPtr<RunObject, std::string> run_data_mapper_;
        DataMapperPtr<RunStepObject, std::string> run_step_data_mapper_;
        DataMapperPtr<MessageObject, std::string> message_data_mapper_;
        // DataMapperPtr<AssistantObject, std::string> assistant_data_mapper_;
        CommonTaskSchedulerPtr task_scheduler_;
        StateManagerPtr state_manager_;
    public:
        RunServiceImpl(const DataMapperPtr<ThreadObject, std::string> &thread_data_mapper,
            const DataMapperPtr<RunObject, std::string> &run_data_mapper,
            const DataMapperPtr<RunStepObject, std::string> &run_step_data_mapper,
            const DataMapperPtr<MessageObject, std::string>& message_data_mapper,
            const StateManagerPtr& state_manager,
            const CommonTaskSchedulerPtr& task_scheduler
            )
            : thread_data_mapper_(thread_data_mapper),
              run_data_mapper_(run_data_mapper),
              run_step_data_mapper_(run_step_data_mapper),
              message_data_mapper_(message_data_mapper),
              state_manager_(state_manager),
              task_scheduler_(task_scheduler) {
        }

        std::optional<RunObject> CreateThreadAndRun(const CreateThreadAndRunRequest &create_thread_and_run_request) override {
            // TODO with transaction
            const auto& thread_object = create_thread_and_run_request.thread();
            assert_not_blank(create_thread_and_run_request.assistant_id(), "should provide assistant id");
            assert_true(create_thread_and_run_request.has_thread(), "should provide thread data");
            assert_gt(thread_object.messages_size(), 0, "should have thread messages");

            // validate message sequence.
            // 1. First and last message should be user message.
            // 2. Two sequential messages of same kind are forbidden.
            bool is_user_message = false;
            for(const auto& msg: create_thread_and_run_request.thread().messages()) {
                if (msg.role() == user) {
                    assert_true(!is_user_message, "illegal sequence of user messages");
                }
                if (msg.role() == assistant) {
                    assert_true(is_user_message, "illegal sequence of assistan messages");
                }
                is_user_message = msg.role() == user;
            }

            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(create_thread_and_run_request, context, {.keep_default_values = true});
            const auto run_id = details::generate_next_object_id("run");

            // create thread
            const auto thread_id = details::generate_next_object_id("thread");;
            context["thread"]["id"] = thread_id;
            EntitySQLUtils::InsertOneThread(thread_data_mapper_, context["thread"]);

            // create messages
            SQLContext insert_messages_context;
            insert_messages_context["messages"] = context["thread"]["messages"];
            for (auto& msg_obj: insert_messages_context["messages"]) {
                msg_obj["id"] = details::generate_next_object_id("message");
                msg_obj["thread_id"] = thread_id;
                if (!msg_obj.contains("status")) {
                    msg_obj["status"] = "completed";
                }
                msg_obj["run_id"] = run_id;
            }
            const auto msg_ids = EntitySQLUtils::InsertManyMessages(message_data_mapper_, insert_messages_context);
            assert_true(msg_ids.size() == thread_object.messages_size(), "should have saved all additional messages");

            // create run
            context["id"] = run_id;
            context["status"] = "queued";
            context["response_format"] = "auto";
            context["truncation_strategy"] = nlohmann::ordered_json::parse(R"({"type":"auto"})");
            context["thread_id"] = thread_id;
            EntitySQLUtils::InsertOneRun(run_data_mapper_, context);

            // get run object
            GetRunRequest get_run_request;
            get_run_request.set_run_id(run_id);
            get_run_request.set_thread_id(thread_id);
            const auto run_object =  RetrieveRun(get_run_request);

            if (run_object.has_value()) {
                // create empty state
                AgentState initial_state;
                initial_state.set_id(run_id);
                state_manager_->Save(initial_state);

                // kick off agent execution
                task_scheduler_->Enqueue({
                    .task_id = run_id,
                    .category = RunObjectTaskHandler::CATEGORY,
                    .payload = ProtobufUtils::Serialize(run_object.value())
                });
            }

            // retrun
            return run_object;
        }

        std::optional<RunObject> CreateRun(const CreateRunRequest &create_request) override {
            // TODO with transaction
            assert_not_blank(create_request.thread_id(), "should provide thread id");
            assert_not_blank(create_request.assistant_id(), "should provide assistant id");
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(create_request, context, {.keep_default_values = true});
            const auto run_id = details::generate_next_object_id("run");

            // create additional messages
            if (create_request.additional_messages_size() > 0) {
                for(auto& msg_obj: context["additional_messages"]) {
                    msg_obj["id"] = details::generate_next_object_id("message");
                    msg_obj["thread_id"] = create_request.thread_id();
                    msg_obj["run_id"] = run_id;
                    if (!msg_obj.contains("status")) {
                        msg_obj["status"] = "completed";
                    }
                }
                SQLContext insert_messages_context;
                insert_messages_context["messages"] = context["additional_messages"];
                const auto msg_ids = EntitySQLUtils::InsertManyMessages(message_data_mapper_, insert_messages_context);
                assert_true(msg_ids.size() == create_request.additional_messages_size(), "should have saved all additional messages");
            }

            // TODO handle additional_instructions and instructions

            // create run
            context["id"] = run_id;
            context["status"] = "queued";
            context["response_format"] = "auto";
            context["truncation_strategy"] = nlohmann::ordered_json::parse(R"({"type":"auto"})");
            EntitySQLUtils::InsertOneRun(run_data_mapper_, context);

            // create empty state
            AgentState initial_state;
            initial_state.set_id(run_id);
            state_manager_->Save(initial_state);

            // start agent exeuction
            task_scheduler_->Enqueue({
                .task_id = run_id,
                .category = RunObjectTaskHandler::CATEGORY
            });

            // return
            GetRunRequest get_run_request;
            get_run_request.set_run_id(run_id);
            get_run_request.set_thread_id(create_request.thread_id());
            return RetrieveRun(get_run_request);
        }

        ListRunsResponse ListRuns(const ListRunsRequest &list_request) override {
            assert_not_blank(list_request.thread_id(), "should provide thread_id");
            SQLContext context;
            // plus one for remaining check
            const auto limit = list_request.limit() <= 0 ? DEFAULT_LIST_LIMIT + 1: list_request.limit() + 1;
            context["limit"] = limit;
            ProtobufUtils::ConvertMessageToJsonObject(list_request, context);
            const auto run_list = EntitySQLUtils::SelectManyRuns(run_data_mapper_, context);

            const auto n = run_list.size();
            ListRunsResponse list_runs_response;
            list_runs_response.set_object("list");
            if (n > limit) {
                list_runs_response.set_has_more(true);
                list_runs_response.set_first_id(run_list.front().id());
                list_runs_response.set_last_id(run_list.at(n-2).id());
                list_runs_response.mutable_data()->Add(run_list.begin(), run_list.end()-1);
            } else {
                list_runs_response.set_has_more(false);
                list_runs_response.set_first_id(run_list.front().id());
                list_runs_response.set_last_id(run_list.back().id());
                list_runs_response.mutable_data()->Add(run_list.begin(), run_list.end());
                // do nothing if n==0
            }
            return list_runs_response;
        }

        std::optional<RunObject> RetrieveRun(const GetRunRequest &get_request) override {
            assert_not_blank(get_request.thread_id(), "should provide thread id");
            assert_not_blank(get_request.run_id(), "should provide run id");
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(get_request, context);
            return EntitySQLUtils::SelectOneRun(run_data_mapper_, context);
        }

        std::optional<RunObject> ModifyRun(const ModifyRunRequest &modify_run_request) override {
            // update
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(modify_run_request, context);
            const auto count = EntitySQLUtils::UpdateRun(run_data_mapper_, context);
            assert_true(count == 1, "should have run object updated");

            // return
            GetRunRequest get_run_request;
            get_run_request.set_thread_id(modify_run_request.thread_id());
            get_run_request.set_run_id(modify_run_request.run_id());
            return RetrieveRun(get_run_request);
        }

        std::optional<RunObject> SubmitToolOutputs(const SubmitToolOutputsToRunRequest &sub_request) override {
            const auto& run_id = sub_request.run_id();
            const auto& thread_id = sub_request.thread_id();
            assert_not_blank(run_id, "should provide run_id");
            assert_not_blank(thread_id, "should provide thread_id");

            // check run status
            GetRunRequest get_run_request;
            get_run_request.set_thread_id(thread_id);
            get_run_request.set_run_id(run_id);
            const auto run_object = RetrieveRun(get_run_request);
            assert_true(run_object.has_value(), fmt::format("should have found run object with thread_id={}, run_id={}", thread_id, run_id));
            assert_true(run_object->status() == RunObject_RunObjectStatus_requires_action, "run object should be in status of 'requires_action'.");
            assert_true(run_object->required_action().type() == RunObject_RequiredActionType_submit_tool_outputs, fmt::format("required action for run object should be type of 'submit_tool_outputs'. thread_id={}, run_id={}", thread_id, run_id));

            // check run last run step
            ListRunStepsRequest list_run_steps_request;
            list_run_steps_request.set_thread_id(thread_id);
            list_run_steps_request.set_run_id(run_id);
            list_run_steps_request.set_order(desc);
            list_run_steps_request.set_limit(1);
            const auto run_step_list_response = ListRunSteps(list_run_steps_request);
            assert_true(run_step_list_response.data_size() == 1, fmt::format("should have found latest run step for thread_id={}, run_id={}", thread_id, run_id));
            const auto &last_run_step = run_step_list_response.data(0);
            assert_true(last_run_step.status() == RunStepObject_RunStepStatus_in_progress, fmt::format("Last run step should be in status of 'in_progress'."));

            // do a copy and filter
            auto copied_run_step = last_run_step;
            auto function_step_details_view = (*copied_run_step.mutable_step_details()->mutable_tool_calls()) | std::views::filter([](const RunStepObject_RunStepDetails_ToolCallDetail& detail) {
                    return detail.type() == function;
            });
            auto size = std::ranges::distance(function_step_details_view);
            assert_true(size > 0, fmt::format("should have at at least one tool call for thread_id={}, run_id={}, run_step_id={}", thread_id, run_id, last_run_step.id()));

            // collect call ids
            auto function_Step_details_ids_view = function_step_details_view | std::views::transform([](const RunStepObject_RunStepDetails_ToolCallDetail& detail) {
                return detail.id();
            });
            std::unordered_set<std::string> function_call_ids {function_Step_details_ids_view.begin(), function_Step_details_ids_view.end()};

            // set tool output result in step_details
            for(const auto& tool_output: sub_request.tool_outputs()) {
                for(auto& function_step_detail: function_step_details_view) {
                    if (function_step_detail.id() == tool_output.tool_call_id()) {
                        function_call_ids.erase(function_step_detail.id());
                        function_step_detail.mutable_function()->set_output(tool_output.output());
                    }
                }
            }
            // check if all function calls are completed
            assert_true(function_call_ids.empty(), fmt::format("should provide results for all function calls. thread_id={}, run_id={}, run_step_id={}, tool_call_ids={}", thread_id, run_id, last_run_step.id(), StringUtils::JoinWith(function_call_ids, ",")));

            // update run step object
            SQLContext update_run_step_object_context;
            ProtobufUtils::ConvertMessageToJsonObject(copied_run_step, update_run_step_object_context);
            auto count = EntitySQLUtils::UpdateRunStep(run_step_data_mapper_, update_run_step_object_context);
            assert_true(count == 1, fmt::format("should have run step object updated. thread_id={}, run_id={}, run_step_id={}", thread_id, run_id, last_run_step.id()));

            // resume agent execution
            task_scheduler_->Enqueue({
                .task_id = run_id,
                .category = RunObjectTaskHandler::CATEGORY
            });

            // update run object
            // status is updated just to signal the change. in fact, actual handling is implemented in `RunObjectTaskHandler`.
            SQLContext update_run_object_context;
            update_run_object_context["thread_id"] = thread_id;
            update_run_object_context["run_id"] = run_id;
            update_run_object_context["status"] = "in_progress";
            count = EntitySQLUtils::UpdateRun(run_data_mapper_, update_run_object_context);
            assert_true(count == 1, fmt::format("should have run object updated. thread_id={}, run_id={}, run_step_id={}", thread_id, run_id, last_run_step.id()));

            // return
            return RetrieveRun(get_run_request);
        }

        std::optional<RunObject> CancelRun(const CancelRunRequest &cancel_request) override {
            // update database only
            // task handler will checkout status in loop and abort if run is cancelled
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(cancel_request, context);
            context["status"] = "cancelling";
            EntitySQLUtils::UpdateRun(run_data_mapper_, context);

            // return
            GetRunRequest get_run_request;
            get_run_request.set_thread_id(cancel_request.thread_id());
            get_run_request.set_run_id(cancel_request.run_id());
            return RetrieveRun(get_run_request);
        }

        ListRunStepsResponse ListRunSteps(const ListRunStepsRequest &list_run_steps_request) override {
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(list_run_steps_request, context);
            auto limit = list_run_steps_request.limit() <= 0 ? DEFAULT_LIST_LIMIT + 1 : list_run_steps_request.limit() + 1;
            context["limit"] = limit;
            if (list_run_steps_request.limit() == unknown_list_request_order) {
                context["order"] = "desc";
            }
            const auto run_step_list = EntitySQLUtils::SelectManyRunSteps(run_step_data_mapper_, context);

            ListRunStepsResponse list_run_steps_response;
            list_run_steps_response.set_object("list");
            if (const auto n = run_step_list.size(); n>limit) {
                list_run_steps_response.set_first_id(run_step_list.front().id());
                list_run_steps_response.set_last_id(run_step_list[n-2].id());
                list_run_steps_response.mutable_data()->Add(run_step_list.begin(), run_step_list.end());
                list_run_steps_response.set_has_more(true);
            } else {
                list_run_steps_response.set_has_more(false);
                list_run_steps_response.mutable_data()->Add(run_step_list.begin(), run_step_list.end());
                list_run_steps_response.set_first_id(run_step_list.front().id());
                list_run_steps_response.set_last_id(run_step_list.back().id());
            }
            return list_run_steps_response;
        }

        std::optional<RunStepObject> GetRunStep(const GetRunStepRequest &get_run_step_request) override {
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(get_run_step_request, context);
            return EntitySQLUtils::GetRunStep(run_step_data_mapper_, context);
        }

        std::optional<RunStepObject> CreateRunStep(const RunStepObject &create_request) override {
            assert_not_blank(create_request.run_id(), "should provide run_id");
            assert_not_blank(create_request.thread_id(), "should provide thread_id");

            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(create_request, context);
            const auto run_step_id = details::generate_next_object_id("run_step");;
            context["id"] = run_step_id;
            EntitySQLUtils::CreateRunStep(run_step_data_mapper_, context);
            GetRunStepRequest get_run_step_request;
            get_run_step_request.set_run_id(create_request.run_id());
            get_run_step_request.set_step_id(run_step_id);
            get_run_step_request.set_thread_id(create_request.thread_id());
            return GetRunStep(get_run_step_request);
        }

        std::optional<RunStepObject> ModifyRunStep(const ModifyRunStepRequest &modify_reequest) override {
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(modify_reequest, context);
            EntitySQLUtils::UpdateRunStep(run_step_data_mapper_, context);

            // return
            GetRunStepRequest get_run_step_request;
            get_run_step_request.set_run_id(modify_reequest.run_id());
            get_run_step_request.set_step_id(modify_reequest.step_id());
            get_run_step_request.set_thread_id(modify_reequest.thread_id());
            return GetRunStep(get_run_step_request);
        }
    };
}

#endif //RUNSERVICEIMPL_HPP
