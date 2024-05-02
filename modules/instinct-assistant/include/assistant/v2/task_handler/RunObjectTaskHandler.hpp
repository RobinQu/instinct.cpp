//
// Created by RobinQu on 2024/4/29.
//

#ifndef RUNOBJECTHANDLER_HPP
#define RUNOBJECTHANDLER_HPP

#include "AssistantGlobals.hpp"
#include "agent_executor/BaseAgentExecutor.hpp"
#include "task_scheduler/ThreadPoolTaskScheduler.hpp"
#include "agent/openai_tool/Agent.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;

    class RunObjectTaskHandler final: public CommonTaskScheduler::ITaskHandler {
//         DataMapperPtr<RunObject, std::string> run_data_mapper_;
//         DataMapperPtr<RunStepObject, std::string> run_step_data_mapper_;
        RunServicePtr run_service_;
        MessageServicePtr message_service_;
        ChatModelPtr  chat_model_;
        FunctionToolkitPtr built_in_toolkit_;

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

            AgentState state;
            RecoverAgentState_(run_object, state);
            AgentExecutorPtr executor = BuildAgentExecutor_(run_object);

            // execute possible steps
            executor->Stream(state)
                | rpp::operators::as_blocking()
                | rpp::operators::subscribe([&](const AgentState& current_state) {
                    const auto last_step = current_state.previous_steps().rbegin();

                    if (last_step->has_thought()
                        && last_step->thought().has_continuation()
                        && last_step->thought().continuation().openai().has_tool_call_message()) { // should contain thought of calling code interpreter and file search, which are invoked automatically
                        OnAgentContinuation(last_step->thought().continuation());
                        return;
                    }

                    if (last_step->has_thought()
                        && last_step->thought().has_pause()
                        && last_step->thought().pause().has_openai()) { // should contain thought of calling function tools
                        OnAgentPause_(last_step->thought().pause());
                        return;
                    }

                    if (last_step->has_observation() && last_step->observation().has_openai()) {
                        // 1. function tool call results are submitted
                        // 2. or only contain tool calls for code interpreter and file search
                        OnAgentObservation(last_step->observation());
                        return;
                    }


                    if(last_step->has_thought() && last_step->thought().has_finish()) { // finish message
                        OnAgentFinish_(last_step->thought().finish());
                        return;
                    }

                    // should be observation which cannot be generated from tool agent. they should be submitted by users through `IRunService::SubmitToolOutputs` and agent executor will consult planer to generate thought messages or finish messages.
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
         * 1. run step object should be created with type of `tool_calls` and status of `in_progress`.
         * 2. update run object to status of `in_progress`.
         * @param agent_continuation
         */
        void OnAgentContinuation(const AgentContinuation& agent_continuation) {

        }


        /**
         * After pause message is generated,
         * 1. update `step_details` of run step object with completed tool call results.
         * 2. update `run_object` with status of `required_action` and correct content of `required_actions`
         * @param agent_pause
         */
        void OnAgentPause_(const AgentPause& agent_pause) {

        }

        /**
         * 1. update run object to status of `in_progress` in case it's previously `required_action`.
         * 2. update of run step object with `step_details` of completed tool call results.
         * @param observation
         */
        void OnAgentObservation(const AgentObservation& observation) {

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
         */
        void OnAgentFinish_(const AgentFinish& finish_message) {

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
         * @param state
         */
        void RecoverAgentState_(const RunObject& run_object, AgentState& state) {
            ListRunStepsRequest list_run_steps_request;
            list_run_steps_request.set_order(asc);
            list_run_steps_request.set_run_id(run_object.id());
            list_run_steps_request.set_thread_id(run_object.thread_id());
            const auto list_run_steps_resp = run_service_->ListRunSteps(list_run_steps_request);

            ListMessageRequest list_message_request;
            list_run_steps_request.set_thread_id(run_object.thread_id());
            list_run_steps_request.set_order(asc);
            auto list_message_response = message_service_->ListMessages(list_message_request);

            std::unordered_map<std::string, MessageObject*> msgs_by_id;
            MessageObject* last_user_message = nullptr;
            for (int i=0; i < list_message_response.data_size(); ++i) {
                auto msg = list_message_response.mutable_data(i);
                msgs_by_id[msg->id()] = msg;
                if (msg->role() == user) {
                    last_user_message = msg;
                }
            }

            assert_true(last_user_message, "Should have user message in thread");

            auto user_message = state.mutable_input()->mutable_chat()->add_messages();
            // TODO support image message
            user_message->set_content(last_user_message->content().text().value());
            user_message->set_role("user");
            for(const auto &step: list_run_steps_resp.data()) {
                // state.mutable_previous_steps()->
                if (step.type() == RunStepObject_RunStepType_message_creation && step.step_details().has_message_creation()) {
                    auto msg = msgs_by_id.at(step.step_details().message_creation().message_id());

                }
                if (step.type() == RunStepObject_RunStepType_tool_calls) {

                }
            }

        }
    };
}


#endif //RUNOBJECTHANDLER_HPP
