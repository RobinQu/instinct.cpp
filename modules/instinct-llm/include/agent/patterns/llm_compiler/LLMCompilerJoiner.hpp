//
// Created by RobinQu on 2024/5/11.
//

#ifndef LLMCOMPILERJOINER_HPP
#define LLMCOMPILERJOINER_HPP

#include "LLMCompilerJoinerResultOutputParser.hpp"
#include "LLMCompilerJoinerTaskGraphInputParser.hpp"
#include "LLMCompilerPlanerThoughtOutputParser.hpp"
#include "LLMGlobals.hpp"
#include "chain/MessageChain.hpp"
#include "chat_model/BaseChatModel.hpp"
#include "prompt/IChatPromptTemplate.hpp"
#include "prompt/PlainChatPromptTemplate.hpp"

namespace INSTINCT_LLM_NS {
    using JoinerPtr = RunnablePtr<LLMCompilerTaskGraph, LLMCompilerJoinerResult>;

    static JoinerPtr CreateLLMCompilerJoiner(
        const ChatModelPtr& chat_model,
        PromptTemplatePtr chat_prompt_template = nullptr,
        const InputParserOptions& input_parser_options = {},
        const LLMCompilerJoinerResultOutputParserOptions& output_parser_options = {}
    ) {
        if(chat_prompt_template == nullptr) {
            chat_prompt_template = CreatePlainChatPromptTemplate({
                    {kHuman, R"(Solve a question answering task. Here are some guidelines:
- In the Assistant Scratchpad, you will be given results of a plan you have executed to answer the user's question.
- Thought needs to reason about the question based on the Observations in 1-2 sentences.
- Ignore irrelevant action results.
- If the required information is present, give a concise but complete and helpful answer to the user's question.
- If you are unable to give a satisfactory finishing answer, replan to get the required information.

Respond in the following format:

Thought: <reason about the task results and whether you have sufficient information to answer the question>
Action: <action to take>(<final answer>)

You should choose one of available actions:
 (1) Finish(the final answer to return to the user): returns the answer and finishes the task.
 (2) Replan(the reasoning and other information that will help you plan again. Can be a line of any length): instructs why we must replan

{messages}


Question: {question}

{agent_scrathpad}

)"}
            });
        }
        const auto input_parser = CreateLLMCompilerJoinerTaskGraphInputParser(input_parser_options);
        const auto output_parser = CreateLLMCompilerJoinerResultOutputParser(output_parser_options);
        return CreateFunctionalChain(input_parser, output_parser, chat_prompt_template | chat_model->AsModelFunction());
    }

}

#endif //LLMCOMPILERJOINER_HPP
