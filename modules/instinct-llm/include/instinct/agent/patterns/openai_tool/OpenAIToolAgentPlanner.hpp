//
// Created by RobinQu on 2024/5/13.
//

#ifndef OPENAITOOLAGENTPLANNER_HPP
#define OPENAITOOLAGENTPLANNER_HPP
#include <instinct/LLMGlobals.hpp>
#include <instinct/agent/executor/IAgentExecutor.hpp>
#include <instinct/chat_model/BaseChatModel.hpp>
#include <instinct/functional/runnable.hpp>

namespace INSTINCT_LLM_NS {
    class OpenAIToolAgentPlanner final: public BaseRunnable<AgentState, AgentThought> {
        ChatModelPtr chat_model_;
    public:
        explicit OpenAIToolAgentPlanner(ChatModelPtr chatModel) : chat_model_(std::move(chatModel)) {}

        AgentThought Invoke(const AgentState &state) override {
            chat_model_->BindToolSchemas({state.function_tools().begin(), state.function_tools().end()});

            // this should contain the first message from user
            PromptValue prompt_value = state.input();
            for (const auto &step: state.previous_steps()) {
                // add tool call message ChatCompletion api
                if (step.has_thought()
                    && step.thought().has_continuation()
                    && step.thought().continuation().has_tool_call_message()) {
                    prompt_value.mutable_chat()->add_messages()->CopyFrom(
                            step.thought().continuation().tool_call_message()
                    );
                }
                if (step.has_observation() && step.observation().
                        tool_messages_size() > 0) {
                    // add tool call results
                    for (const auto &msg: step.observation().tool_messages()) {
                        prompt_value.mutable_chat()->add_messages()->CopyFrom(msg);
                    }
                }
            }

            const auto message = chat_model_->Invoke(prompt_value);
            AgentThought thought_message;
            if (message.tool_calls_size() > 0) { // has more tool call requests
                thought_message.mutable_continuation()->mutable_tool_call_message()->CopyFrom(message);
                return thought_message;
            }
            thought_message.mutable_finish()->set_response(message.content());
            return thought_message;
        }
    };

    /**
     * Create OpenAI style tool agent
     * @param chat_model A chat model provider that supports tool calling APIs
     * @return
     */
    static PlannerPtr CreateOpenAIToolAgentPlanner(const ChatModelPtr &chat_model) {
        return std::make_shared<OpenAIToolAgentPlanner>(chat_model);
    }
}
#endif //OPENAITOOLAGENTPLANNER_HPP
