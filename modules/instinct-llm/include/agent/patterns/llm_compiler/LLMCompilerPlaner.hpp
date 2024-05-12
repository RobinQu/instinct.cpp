//
// Created by RobinQu on 2024/5/11.
//

#ifndef LLMCOMPILERPLANER_HPP
#define LLMCOMPILERPLANER_HPP
#include "LLMGlobals.hpp"
#include "agent/executor/IAgentExecutor.hpp"
#include "chat_model/BaseChatModel.hpp"
#include "prompt/PlainChatPromptTemplate.hpp"


namespace INSTINCT_LLM_NS {
    /**
     * named context variables are:
     * 1. question: user input
     * 2. num_tools: number of tools
     * 3. tool_descriptions: fomrated list of tool descriptions
     * 4. replan_instruction: re-planing instruction if applicable
     * 5. context: context for re-planing if applicable
     *
     * Implmentations:
     * 1. if last step doesn't exist, then let's do first plan
     * 2. if last step has observation (except join), we do join
     * 2.1 if `join` gives out final result, we return thought with final message
     * 2.2 if `join` gives out replan request, we run LLM with replan prompt.
     */
    class LLMCompilerAgnetStateInputParser final: public BaseInputParser<AgentState> {
    public:
        JSONContextPtr ParseInput(const AgentState &agent_state) override {
            const int n = agent_state.previous_steps_size();
            std::string prompt_input = MessageUtils::ExtractLatestPromptString(agent_state.input());

            // generate tool descriptions
            std::string tool_descriptions;
            for (int i=0; i<agent_state.function_tools_size(); ++i) {
                const auto& function_tool = agent_state.function_tools(i);
                tool_descriptions += fmt::format("{}. {}: {}, arguments JSON schema: {}",  function_tool.name(), function_tool.description(), ProtobufUtils::Serialize(function_tool.parameters()));
            }

            // return context
            return CreateJSONContext({
                {"question", prompt_input},
                // plus for for extra `join` function
                {"num_tools", agent_state.function_tools_size() + 1 },
                {"tool_descriptions", tool_descriptions},
                {"replan_instruction", ""},
                {"context", ""}
            });
        }
    };

    static InputParserPtr<AgentState> CreateLLMCompilerInputParser() {


    }


    class LLMCompilerThoughtOutputParser final: public BaseOutputParser<AgentThought> {
    public:
        AgentThought ParseResult(const Generation &context) override;
    };

    static OutputParserPtr<AgentThought> CreateLLMCompilerOutputParser() {

    }

    static PlannerPtr CreateLLMCompilerPlaner(
        const ChatModelPtr &chat_model,
        const std::vector<FunctionToolkitPtr> &built_in_toolkits,
        PromptTemplatePtr prompt_template = nullptr
    ) {
        if (!prompt_template) {
            prompt_template = CreatePlainChatPromptTemplate({
                {
                kHuman,
                R"(Given a user query, create a plan to solve it with the utmost parallelization. Each plan should comprise an action from the following {num_tools} types:
    {tool_descriptions}
    {num_tools}. join(): Collects and combines results from prior actions. No arguments needed.

     - An LLM agent is called upon invoking join() to either finalize the user query or wait until the plans are executed.
     - join should always be the last action in the plan, and will be called in two scenarios:
       (a) if the answer can be determined by gathering the outputs from tasks to generate the final response.
       (b) if the answer cannot be determined in the planning phase before you execute the plans. Guidelines:
     - Each action described above contains input/output types and description.
        - You must strictly adhere to the input and output types for each action.
        - The action descriptions contain the guidelines. You MUST strictly follow those guidelines when you use the actions.
     - Each action in the plan should strictly be one of the above types.
     - Each action MUST have a unique ID, which is strictly increasing.
     - Input to the action is formatted as JSON blob with 'name' and 'arguments' keys.
     - If inputs for actions are outputs from preceding actions,  always use the format $id to denote the ID of the previous action whose output will be used as the input.
     - Always call join as the last action in the plan. Say '<END_OF_PLAN>' after you call join in a new line.
     - Ensure the plan maximizes parallelization.
     - Only use the provided action types. If a query cannot be addressed using these, invoke the join action for the next steps.
     - Never introduce new actions other than the ones provided.

    Remember, ONLY respond with the task list in the following format:
    ID. JSON blob of action input

    {replan_instruction}

    Question: {question}

    {context})"
                }
            });
        }
        //

        return CreateFunctionalChain();

    }
}

#endif //LLMCOMPILERPLANER_HPP
