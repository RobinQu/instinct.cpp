//
// Created by RobinQu on 2024/4/26.
//

#ifndef RUNSERVICEIMPL_HPP
#define RUNSERVICEIMPL_HPP

#include "../IRunService.hpp"
#include "assistant/v2/tool/EntitySQLUtils.hpp"
#include "database/IDataMapper.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;

    class RunServiceImpl final: public IRunService {
        DataMapperPtr<ThreadObject, std::string> thread_data_mapper_;
        DataMapperPtr<RunObject, std::string> run_data_mapper_;
        DataMapperPtr<RunStepObject, std::string> run_step_data_mapper_;
    public:
        RunServiceImpl(const DataMapperPtr<ThreadObject, std::string> &thread_data_mapper,
            const DataMapperPtr<RunObject, std::string> &run_data_mapper,
            const DataMapperPtr<RunStepObject, std::string> &run_step_data_mapper)
            : thread_data_mapper_(thread_data_mapper),
              run_data_mapper_(run_data_mapper),
              run_step_data_mapper_(run_step_data_mapper) {
        }

        std::optional<RunObject> CreateThreadAndRun(const CreateThreadAndRunRequest &create_thread_and_run_request) override {
            assert_true(create_thread_and_run_request.has_thread(), "should provide thread data");

            // TODO with transaction
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(create_thread_and_run_request, context);

            // create thread
            // const auto& thread_object = create_thread_and_run_request.thread();
            const auto thread_id = details::generate_next_object_id("thread");;
            context["thread"]["id"] = thread_id;
            EntitySQLUtils::InsertOneThread(thread_data_mapper_, context["thread"]);

            // create run
            const auto run_id = details::generate_next_object_id("run");;
            context["id"] = run_id;
            EntitySQLUtils::InsertOneRun(run_data_mapper_, context);

            // TODO submit run to task queue

            // return
            GetRunRequest get_run_request;
            get_run_request.set_run_id(run_id);
            return RetrieveRun(get_run_request);
        }

        std::optional<RunObject> CreateRun(const CreateRunRequest &create_request) override {
            // create run
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(create_request, context);
            const auto run_id = details::generate_next_object_id("run");;
            context["id"] = run_id;
            EntitySQLUtils::InsertOneRun(run_data_mapper_, context);

            // return
            GetRunRequest get_run_request;
            get_run_request.set_run_id(run_id);
            return RetrieveRun(get_run_request);
        }

        ListRunsResponse ListRuns(const ListRunsRequest &list_request) override {
            assert_not_blank(list_request.thread_id(), "should provide thread_id");
            SQLContext context;
            // plus one for remianing check
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
                if(n>1) {
                    list_runs_response.set_first_id(run_list.front().id());
                    list_runs_response.set_last_id(run_list.at(n-2).id());
                    list_runs_response.mutable_data()->Add(run_list.begin(), run_list.end()-1);
                } else if(n==1) {
                    list_runs_response.set_first_id(run_list.front().id());
                    list_runs_response.set_last_id(run_list.front().id());
                    list_runs_response.add_data()->CopyFrom(run_list.front());
                }
                // do nothing if n==0
            }
            return list_runs_response;
        }

        std::optional<RunObject> RetrieveRun(const GetRunRequest &get_request) override {
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
            assert_true(run_object->required_action().type() == RunObject_RequiredAction::submit_tool_outputs, fmt::format("required action for run object should be type of 'submit_tool_outputs'. thread_id={}, run_id={}", thread_id, run_id));

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
            auto function_step_detaiils_view = (*copied_run_step.mutable_step_details()->mutable_tool_calls()) | std::views::filter([](const RunStepObject_RunStepDetails_ToolCallDetail& detail) {
                    return detail.type() == function;
            });
            auto size = std::ranges::distance(function_step_detaiils_view);
            assert_true(size > 0, fmt::format("should have at at least one tool call for thread_id={}, run_id={}, run_step_id={}", thread_id, run_id, last_run_step.id()));

            // collect call ids
            auto function_Step_details_ids_view = function_step_detaiils_view | std::views::transform([](const RunStepObject_RunStepDetails_ToolCallDetail& detail) {
                return detail.id();
            });
            std::unordered_set<std::string> function_call_ids {function_Step_details_ids_view.begin(), function_Step_details_ids_view.end()};

            // set tool ouptut result in step_details
            for(const auto& tool_output: sub_request.tool_outputs()) {
                for(auto& function_step_detail: function_step_detaiils_view) {
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

            // TODO resume agent execution

            // update run object
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
            // update database
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(cancel_request, context);
            context["status"] = "cancelling";
            EntitySQLUtils::UpdateRun(run_data_mapper_, context);

            // TODO cancel in task queue

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
                if (n>1) {
                    list_run_steps_response.mutable_data()->Add(run_step_list.begin(), run_step_list.end());
                    list_run_steps_response.set_first_id(run_step_list.front().id());
                    list_run_steps_response.set_last_id(run_step_list.back().id());
                } else if(n==1) {
                    list_run_steps_response.mutable_data()->Add()->CopyFrom(run_step_list.front());
                    list_run_steps_response.set_first_id(run_step_list.front().id());
                    list_run_steps_response.set_last_id(run_step_list.front().id());
                }
                // do nothing if n == 0
            }
            return list_run_steps_response;
        }

        std::optional<RunStepObject> GetRunStep(const GetRunStepRequest &get_run_step_request) override {
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(get_run_step_request, context);
            return EntitySQLUtils::GetRunStep(run_step_data_mapper_, context);
        }
    };
}

#endif //RUNSERVICEIMPL_HPP
