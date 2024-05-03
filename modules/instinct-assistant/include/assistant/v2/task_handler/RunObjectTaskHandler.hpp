//
// Created by RobinQu on 2024/4/29.
//

#ifndef RUNOBJECTHANDLER_HPP
#define RUNOBJECTHANDLER_HPP

#include "AssistantGlobals.hpp"
#include "agent/executor/BaseAgentExecutor.hpp"
#include "task_scheduler/ThreadPoolTaskScheduler.hpp"
#include "agent/patterns/openai_tool/Agent.hpp"
#include "agent/state/IStateManager.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;

    class RunObjectTaskHandler final: public CommonTaskScheduler::ITaskHandler {
//         DataMapperPtr<RunObject, std::string> run_data_mapper_;
//         DataMapperPtr<RunStepObject, std::string> run_step_data_mapper_;
        RunServicePtr run_service_;
        MessageServicePtr message_service_;
        AssistantServicePtr assistant_service_;
        ChatModelPtr  chat_model_;
        FunctionToolkitPtr built_in_toolkit_;
        StateManagerPtr state_manager_;


    public:
        static inline std::string CATEGORY = "run_object";

        bool Accept(const ITaskScheduler<std::string>::Task &task) override {
            return task.category == CATEGORY;
        }

        void Handle(const ITaskScheduler<std::string>::Task &task) override {
            RunObject run_object;
            ProtobufUtils::Deserialize(task.payload, run_object);

            if (CheckPreconditions_(run_object)) {
                LOG_WARN("Precondition failure for run object: {}", run_object.ShortDebugString());
                return;
            }

            const auto state_opt = RecoverAgentState_(run_object);
            if (!state_opt.has_value()) {
                LOG_ERROR("Failed to recover state with run object: {}", run_object.ShortDebugString());
                return;
            }

            const auto& state = state_opt.value();
            const auto executor = BuildAgentExecutor_(run_object);

            // execute possible steps
            executor->Stream(state)
                | rpp::operators::as_blocking()
                | rpp::operators::subscribe([&](const AgentState& current_state) {
                    // persist current state
                    state_manager_->Save(current_state);

                    // respond to state changes
                    const auto last_step = current_state.previous_steps().rbegin();
                    if (last_step->has_thought()) {
                        if (last_step->thought().has_continuation()
                            && last_step->thought().continuation().openai().has_tool_call_message()) { // should contain thought of calling code interpreter and file search, which are invoked automatically
                            OnAgentContinuation(last_step->thought().continuation(), run_object);
                            return;
                        }

                        if (last_step->thought().has_pause()
                            && last_step->thought().pause().has_openai()) { // should contain thought of calling function tools
                            OnAgentPause_(last_step->thought().pause(), run_object);
                            return;
                        }

                        if(last_step->thought().has_finish()) { // finish message
                            OnAgentFinish_(last_step->thought().finish(), run_object);
                            return;
                        }
                    }

                    if (last_step->has_observation() && last_step->observation().has_openai()) {
                        // 1. function tool call results are submitted
                        // 2. or only contain tool calls for code interpreter and file search
                        OnAgentObservation_(last_step->observation(), run_object);
                        return;
                    }

                    LOG_WARN("Illegal message from agent: {}", last_step->ShortDebugString());
                });

        }


    private:

        /**
         * Check following conditions:
         * 1. run object is in status of `in_progress` or `requires_action`.
         * 2. file resources referenced are valid
         * @param run_object
         * @return
         */
        bool CheckPreconditions_(const RunObject& run_object) { // NOLINT(*-convert-member-functions-to-static)
            return run_object.status() == RunObject_RunObjectStatus_in_progress || run_object.status() == RunObject_RunObjectStatus_requires_action;
        }

        /**
         * Just return `OpenAIToolAgentExecutor`
         * @param run_object
         * @return
         */
        AgentExecutorPtr BuildAgentExecutor_(const RunObject& run_object) {
            // no built-in toolkit for now
            return CreateOpenAIToolAgentExecutor(chat_model_, {});
        }


        /**
         * After continuation message is generated,
         * 1. create run step of `message_creation` if `OpenAIToolAgentContinuation.tool_call_message` contains content string
         * 2. create run step object with type of `tool_calls` and status of `in_progress` if `OpenAIToolAgentContinuation.tool_call_message` contains non-empty `tool_calls`.
         * 3. update run object to status of `in_progress`.
         * @param agent_continuation
         * @param run_object
         */
        void OnAgentContinuation(const AgentContinuation& agent_continuation, const RunObject& run_object) {
            LOG_INFO("OnAgentContinuation Start, run_object={}", run_object.ShortDebugString());

            RunStepObject run_step_object;
            run_step_object.set_thread_id(run_object.thread_id());
            run_step_object.set_run_id(run_object.id());
            run_step_object.set_type(RunStepObject_RunStepType_tool_calls);
            run_step_object.set_assistant_id(run_object.assistant_id());
            auto* step_details = run_step_object.mutable_step_details();

            // create step with message if tool message has content string
            if (StringUtils::IsNotBlankString(agent_continuation.openai().tool_call_message().content())) {
                if (const auto resp = CreateMessageStep_(agent_continuation.openai().tool_call_message().content(), run_object);
                    !resp.has_value()) {
                    LOG_ERROR("Illegal reponse for creating step object with message. tool_call_message={}, run_object={}", agent_continuation.openai().tool_call_message().DebugString(), run_object.ShortDebugString());
                    return;
                }
            }

            // create step with tool step if tool message contains tool call requests
            if (agent_continuation.openai().tool_call_message().tool_calls_size() > 0) {
                // TODO support code interpreter and file serach
                for(const auto& tool_request: agent_continuation.openai().tool_call_message().tool_calls()) {
                    auto* tool_call_detail = step_details->mutable_tool_calls()->Add();
                    tool_call_detail->set_id(tool_request.id());
                    tool_call_detail->set_type(function);
                    auto* function_call = tool_call_detail->mutable_function();
                    function_call->set_name(tool_request.function().name());
                    function_call->set_arguments(tool_request.function().arguments());
                }
                if (const auto create_run_resp = run_service_->CreateRunStep(run_step_object);
                    !create_run_resp.has_value()) {
                    LOG_ERROR("Illegal response for creating run step object: {}", run_step_object.ShortDebugString());
                    return;
                    }
            }

            ModifyRunRequest modify_run_request;
            modify_run_request.set_run_id(run_object.id());
            modify_run_request.set_thread_id(run_object.thread_id());
            modify_run_request.set_status(RunObject_RunObjectStatus_in_progress);
            if(const auto modify_run_resp = run_service_->ModifyRun(modify_run_request);
                !modify_run_resp.has_value()) {
                LOG_ERROR("Illegal response for updating run object: {}", modify_run_request.ShortDebugString());
                return;
            }

            LOG_INFO("OnAgentContinuation Done, run_object={}", run_object.ShortDebugString());
        }

        std::optional<std::pair<RunStepObject, MessageObject>> CreateMessageStep_(const std::string& content, const RunObject& run_object) {
            // TODO need transaction
            CreateMessageRequest create_message_request;
            create_message_request.set_thread_id(run_object.thread_id());
            create_message_request.set_role(assistant);
            create_message_request.set_content(content);
            const auto message_object = message_service_->CreateMessage(create_message_request);
            if (!message_object.has_value()) {
                LOG_ERROR("Cannot create message for this step. run_object={}, create_message_request={}", run_object.ShortDebugString(), create_message_request.ShortDebugString());
                return std::nullopt;
            }

            RunStepObject run_step_object;
            run_step_object.set_run_id(run_object.id());
            run_step_object.set_thread_id(run_object.thread_id());
            run_step_object.set_assistant_id(run_object.assistant_id());
            run_step_object.set_type(RunStepObject_RunStepType_message_creation);
            run_step_object.mutable_step_details()->set_type(RunStepObject_RunStepType_message_creation);
            run_step_object.mutable_step_details()->mutable_message_creation()->set_message_id(message_object->id());
            const auto create_run_step_resp = run_service_->CreateRunStep(run_step_object);
            if(!create_run_step_resp.has_value()) {
                LOG_ERROR("Cannot create run step, run_step_object={}", run_step_object.ShortDebugString());
                return std::nullopt;
            }

            return std::pair {create_run_step_resp.value(), message_object.value()};
        }


        /**
         * After pause message is generated,
         * 1. update `step_details` of run step object with completed tool call results.
         * 2. update `run_object` with status of `required_action` and correct content of `required_actions`
         * @param agent_pause
         * @param run_object
         */
        void OnAgentPause_(const AgentPause& agent_pause, const RunObject& run_object) {
            LOG_INFO("OnAgentPause Start, run_object={}", run_object.ShortDebugString());
            const auto last_run_step_opt = RetrieveLastRunStep_(run_object);
            if (!last_run_step_opt.has_value()) {
                LOG_ERROR("Cannot find last run step for run object: {}", run_object.ShortDebugString());
                return;
            }

            // update run step
            ModifyRunStepRequest modify_run_step_request;
            modify_run_step_request.set_run_id(run_object.id());
            modify_run_step_request.set_step_id(last_run_step_opt->id());
            modify_run_step_request.set_thread_id(run_object.thread_id());
            auto* step_details = modify_run_step_request.mutable_step_details();
            step_details->CopyFrom(last_run_step_opt->step_details());

            // TODO support code interpreter and file serach
            for(const auto& tool_message: agent_pause.openai().completed()) {
                for(int i=0;i<step_details->tool_calls_size();++i) {
                    if (auto* tool_call = step_details->mutable_tool_calls(i);
                        tool_call->id() == tool_message.tool_call_id()) {
                        tool_call->mutable_function()->set_output(tool_message.content());
                    }
                }
            }
            if (const auto modify_run_stpe_resp = run_service_->ModifyRunStep(modify_run_step_request);
                !modify_run_stpe_resp.has_value()) {
                LOG_ERROR("Illegal response for updating run step object: {}", modify_run_step_request.ShortDebugString());
                return;
            }

            // update run object
            ModifyRunRequest modify_run_request;
            modify_run_request.set_thread_id(run_object.thread_id());
            modify_run_request.set_run_id(run_object.id());
            modify_run_request.set_status(RunObject_RunObjectStatus_requires_action);
            if(const auto modify_run_resp = run_service_->ModifyRun(modify_run_request); !modify_run_resp.has_value()) {
                LOG_ERROR("Illegal response for update run object: {}", modify_run_request.ShortDebugString());
                return;
            }
            LOG_INFO("OnAgentPause Completed, run_object={}", run_object.ShortDebugString());
        }

        /**
         * 1. update run object to status of `in_progress` in case it's previously `required_action`.
         * 2. update of run step object with `step_details` of completed tool call results.
         * @param observation
         * @param run_object
         */
        void OnAgentObservation_(const AgentObservation& observation, const RunObject& run_object) {
            LOG_INFO("OnAgentObservation Start, run_object={}", run_object.ShortDebugString());

            // update status of run object to `in_progress`
            ModifyRunRequest modify_run_request;
            modify_run_request.set_run_id(run_object.id());
            modify_run_request.set_thread_id(run_object.thread_id());
            modify_run_request.set_status(RunObject_RunObjectStatus_in_progress);

            if (const auto modify_run_resp = run_service_->ModifyRun(modify_run_request); !modify_run_resp.has_value()) {
                LOG_ERROR("Cannot update run object. modify_run_request={}", modify_run_request.ShortDebugString());
                return;
            }

            const auto last_run_step_opt = RetrieveLastRunStep_(run_object);
            if (!last_run_step_opt.has_value()) {
                LOG_ERROR("Cannot find last run step for run object: {}", run_object.ShortDebugString());
                return;
            }

            // update run step with tool call outputs
            ModifyRunStepRequest modify_run_step_request;
            modify_run_step_request.set_run_id(run_object.id());
            modify_run_step_request.set_step_id(last_run_step_opt->id());
            modify_run_step_request.set_thread_id(run_object.thread_id());
            auto* step_details = modify_run_step_request.mutable_step_details();
            step_details->CopyFrom(last_run_step_opt->step_details());

            // TODO support code interpreter and file serach
            for(const auto& tool_message: observation.openai().tool_messages()) {
                for(int i=0;i<step_details->tool_calls_size();++i) {
                    if (auto* tool_call = step_details->mutable_tool_calls(i);
                        tool_call->id() == tool_message.tool_call_id()) {
                        tool_call->mutable_function()->set_output(tool_message.content());
                        }
                }
            }
            if (const auto modify_run_stpe_resp = run_service_->ModifyRunStep(modify_run_step_request);
                !modify_run_stpe_resp.has_value()) {
                LOG_ERROR("Illegal response for updating run step object: {}", modify_run_step_request.ShortDebugString());
                return;
            }
            LOG_INFO("OnAgentObservation Done, run_object={}", run_object.ShortDebugString());
        }

        /**
         * if it's successfully finished:
         * 1. update current run step object with status of `completed`
         * 2. create a new run step object with type of `message_creation`.
         * 3. create a message containing result content in the current thread.
         * 4. update run object with status of `completed`.
         *
         * if it's finished with exception
         * 1. update run step object with status of `failed` and correct content of `last_error`.
         * 2. update run object with status of `failed`.
         *
         * @param finish_message
         * @param run_object
         */
        void OnAgentFinish_(const AgentFinish& finish_message, const RunObject& run_object) {
            LOG_INFO("OnAgentFinish Start, run_object={}", run_object.ShortDebugString());

            const auto last_run_step_opt = RetrieveLastRunStep_(run_object);
            if (!last_run_step_opt.has_value()) {
                LOG_ERROR("Cannot find last run step for run object: {}", run_object.ShortDebugString());
                return;
            }

            // TODO needs transaction

            if (finish_message.has_error()) {
                // update last run step object with status of `failed`
                ModifyRunStepRequest modify_run_step_request;
                modify_run_step_request.set_run_id(run_object.id());
                modify_run_step_request.set_step_id(last_run_step_opt->id());
                modify_run_step_request.set_thread_id(run_object.thread_id());
                modify_run_step_request.set_failed_at(ChronoUtils::GetCurrentTimeMillis());
                modify_run_step_request.set_status(RunStepObject_RunStepStatus_failed);
                // TODO set correct error type
                modify_run_step_request.mutable_last_error()->set_type(invalid_request_error);
                modify_run_step_request.mutable_last_error()->set_message(finish_message.exception());
                if (const auto &modify_run_step_resp = run_service_->ModifyRunStep(modify_run_step_request); !modify_run_step_resp.has_value()) {
                    LOG_ERROR("Failed to update run step object. modify_run_step_request={}", modify_run_step_request.ShortDebugString());
                    return;
                }

                // update run object with status of `failed`
                ModifyRunRequest modify_run_request;
                modify_run_request.set_run_id(run_object.id());
                modify_run_request.set_thread_id(run_object.thread_id());
                modify_run_request.set_status(RunObject_RunObjectStatus_failed);
                if (const auto modify_run_resp = run_service_->ModifyRun(modify_run_request); !modify_run_resp.has_value()) {
                    LOG_ERROR("Failed to update run object. modify_run_request={}", modify_run_request.ShortDebugString());
                    return;
                }
            }
            // update last run step object with status of `completed`
            ModifyRunStepRequest modify_run_step_request;
            modify_run_step_request.set_run_id(run_object.id());
            modify_run_step_request.set_step_id(last_run_step_opt->id());
            modify_run_step_request.set_thread_id(run_object.thread_id());
            modify_run_step_request.set_completed_at(ChronoUtils::GetCurrentTimeMillis());
            modify_run_step_request.set_status(RunStepObject_RunStepStatus_completed);
            if (const auto &modify_run_step_resp = run_service_->ModifyRunStep(modify_run_step_request); !modify_run_step_resp.has_value()) {
                LOG_ERROR("Failed to update run step object. modify_run_step_request={}", modify_run_step_request.ShortDebugString());
                return;
            }

            // create message step
            CreateMessageStep_(finish_message.response(), run_object);


            // update run object with status of `completed`
            ModifyRunRequest modify_run_request;
            modify_run_request.set_run_id(run_object.id());
            modify_run_request.set_thread_id(run_object.thread_id());
            modify_run_request.set_status(RunObject_RunObjectStatus_completed);
            if (const auto modify_run_resp = run_service_->ModifyRun(modify_run_request); !modify_run_resp.has_value()) {
                LOG_ERROR("Failed to update run object. modify_run_request={}", modify_run_request.ShortDebugString());
                return;
            }
            LOG_INFO("OnAgentFinish Done, run_object={}", run_object.ShortDebugString());
        }


        std::optional<RunStepObject> RetrieveLastRunStep_(const RunObject& run_object) {
            ListRunStepsRequest list_run_steps_request;
            list_run_steps_request.set_order(asc);
            list_run_steps_request.set_run_id(run_object.id());
            list_run_steps_request.set_thread_id(run_object.thread_id());
            const auto list_run_steps_resp = run_service_->ListRunSteps(list_run_steps_request);
            if (list_run_steps_resp.data_size() > 0) {
                return *list_run_steps_resp.data().rbegin();
            }
            return std::nullopt;
        }


        /**
         * To find user input:
         * 1. list messages in thread
         * 2. find the latest user message
         * 3. throw if not found
         *
         * To find previous steps:
         * 1. list run steps objects in run object in descendant order in terms of `created_at`.
         * 2. loop each run step
         *      1. For step object with type of `tool_calls`,
         *          1. if it's in status of `in_progress`,
         *              1. create `AgentThought` with `OpenAIToolAgentContinuation`.
         *                  1. if `last_step` is kind of `message_creation`, treat it as `content` of `tool_message`.
         *              2. if its `step_details` contains results of completed all tool calls, create `AgentObservation` with `OpenAIToolAgentObservation`.
         *          2. if it's in status of `required_action`, create `AgentThought` with `OpenAIToolAgentPause`.
         *          3. if it's in status of `failed` or `expired`, create `AgentThought` with `AgentFinish` with corresponding fields.
     *          2. For step object with type of `message_creation`
         *          1. if it's last step and run object is in terminal state (`completed`, `failed`, `expired`), create `AgentFinish` with message content in response field.
         *          2. Or just update `last_step` reference.
         *
         * @param run_object
         * @return state
         */
        std::optional<AgentState> RecoverAgentState_(const RunObject& run_object) {
            auto state = state_manager_->Load(run_object.id()).get();
            const auto last_run_step_object = RetrieveLastRunStep_(run_object);
            if (!last_run_step_object.has_value()) {
                LOG_ERROR("Cannot find last run step for run object: {}", run_object.ShortDebugString());
                return std::nullopt;
            }

            // load function tools
            {
                // create intial agent state
                std::vector<FunctionTool> function_tools;
                // 1. find tools on assistant
                GetAssistantRequest get_assistant_request;
                get_assistant_request.set_assistant_id(run_object.assistant_id());
                const auto assistant = assistant_service_->RetrieveAssistant(get_assistant_request);
                assert_true(assistant.has_value(), "should have found assistant");
                for (const auto& assistant_tool: assistant->tools()) {
                    if (assistant_tool.type() == function) {
                        function_tools.push_back(assistant_tool.function());
                    }
                }

                // 2. find tools on run object
                for (const auto& assistant_tool: run_object.tools()) {
                    if (assistant_tool.type() == function) {
                        function_tools.push_back(assistant_tool.function());
                    }
                }

                // 3. filter and transfrom to function tool schema
                const auto final_function_tools = std::ranges::unique(function_tools, [](const FunctionTool& a, const FunctionTool& b) {
                    return a.name() == b.name();
                });
                LOG_DEBUG("Found {} function tools for agent state, id={}", final_function_tools.size(), state.id());
                for (auto& function_tool: final_function_tools) {
                    state.mutable_function_tools()->Add()->CopyFrom(function_tool);
                }
            }


            // handle pause step
            {
                // fill in complted function call data which is updated by users
                // see instinct::assistant::v2::RunServiceImpl::SubmitToolOutputs
                if (const auto last_step = state.mutable_previous_steps()->rbegin();
                        last_step->has_thought()
                        && last_step->thought().has_pause()
                        && last_step->thought().pause().has_openai()) {
                    if (last_run_step_object->type() == RunStepObject_RunStepType_tool_calls) {
                        // clear completed list first
                        last_step->mutable_thought()->mutable_pause()->mutable_openai()->clear_completed();
                        for(const auto& tool_request: last_step->thought().pause().openai().tool_call_message().tool_calls()) {
                            if (!tool_request.has_function()) {
                                continue;
                            }
                            for(const auto& tool_call: last_run_step_object->step_details().tool_calls()) {
                                if (tool_call.type() == function && tool_request.id() == tool_call.id()) {
                                    // append to completed list if both id and type are matched
                                    auto* tool_message = last_step->mutable_thought()->mutable_pause()->mutable_openai()->mutable_completed()->Add();
                                    tool_message->set_role("tool");
                                    tool_message->set_content(tool_call.function().output());
                                }
                            }
                        }
                        LOG_DEBUG("Found {} completed, {} requested. last_run_step_object={}", last_step->thought().pause().openai().completed_size(), last_step->thought().pause().openai().tool_call_message().tool_calls_size(), last_run_step_object->ShortDebugString());
                    } else {
                        return std::nullopt;
                    }
                }
            }

            return state;
        }
    };
}


#endif //RUNOBJECTHANDLER_HPP
