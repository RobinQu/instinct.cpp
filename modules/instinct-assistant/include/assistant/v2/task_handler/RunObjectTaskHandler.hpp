//
// Created by RobinQu on 2024/4/29.
//

#ifndef RUNOBJECTHANDLER_HPP
#define RUNOBJECTHANDLER_HPP

#include "AssistantGlobals.hpp"
#include "agent_executor/BaseAgentExecutor.hpp"
#include "task_scheduler/ThreadPoolTaskScheduler.hpp"

namespace INSTINCT_ASSISTANT_NS {
    using namespace INSTINCT_DATA_NS;

    class RunObjectTaskHandler final: public CommonTaskScheduler::ITaskHandler {
//         DataMapperPtr<RunObject, std::string> run_data_mapper_;
//         DataMapperPtr<RunStepObject, std::string> run_step_data_mapper_;
        RunServicePtr run_service_;
        MessageServicePtr message_service_;
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
                | rpp::operators::subscribe([&](const AgentState& current_state) {
                    const auto last_step = current_state.previous_steps().rbegin();
                    if (last_step->has_thought() && last_step->thought().has_openai() && last_step->thought().openai().has_tool_call_message()) { // should be thought of openai tool agent
                        CreateRunStep_(last_step->thought().openai());
                        return;
                    }

                    if(last_step->has_finish()) { // finish message
                        CreateRunStep_(last_step->finish());
                        return;
                    }

                    // should be observation which cannot be generated from tool agent. they should be submitted by users through `IRunService::SubmitToolOutputs` and agent executor will consult planer to generate thought messages or finish messages.
                    LOG_WARN("Illegal message from agent: {}", last_step->ShortDebugString());
                });
        }


    private:

        bool CheckPreconditions_(const RunObject& run_object) {

        }

        AgentExecutorPtr BuildAgentExecutor_(const RunObject& run_object) {

        }

        void CreateRunStep_(const AgentFinishStepMessage& finish_message) {

        }

        void CreateRunStep_(const OpenAIToolAgentThoughtMessage& thought_message) {

        }



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
