//
// Created by RobinQu on 2024/5/19.
//

#ifndef LLMOBJECTFACTORY_HPP
#define LLMOBJECTFACTORY_HPP

#include "LLMGlobals.hpp"
#include "agent/patterns/llm_compiler/LLMCompilerAgentExecutor.hpp"
#include "chat_model/BaseChatModel.hpp"
#include "chat_model/OllamaChat.hpp"
#include "chat_model/OpenAIChat.hpp"
#include "commons/OllamaCommons.hpp"
#include "commons/OpenAICommons.hpp"

namespace INSTINCT_LLM_NS {

    // defualt values are required
    struct LLMProviderOptions {
        std::string provider_name = "openai";
        OpenAIConfiguration openai = {};
        OllamaConfiguration ollama = {};
    };

    struct AgentExecutorOptions {
        std::string agent_executor_name = "llm_compiler";
        LLMCompilerOptions llm_compiler = {};
    };


    class LLMObjectFactory final {
    public:

        static ChatModelPtr CreateChatModel(const LLMProviderOptions& options) {
            if (options.provider_name == "ollama") {
                return CreateOllamaChatModel(options.ollama);
            }
            if (options.provider_name == "openai") {
                return CreateOpenAIChatModel(options.openai);
            }
            return nullptr;
        }


        static AgentExecutorPtr CreateAgentExecutor(const AgentExecutorOptions& options, const ChatModelPtr& chat_model, const StopPredicate& predicate) {
            if(options.agent_executor_name == "llm_compiler") {
                // TODO currently build with empty toolkit. we have to add `file-search` and `code-interpreter` in the future
                LOG_INFO("Create LLMCompilerAgentExectuor");
                return CreateLLMCompilerAgentExecutor(chat_model, {}, predicate, options.llm_compiler);
            }
            assert_true(std::dynamic_pointer_cast<OpenAIChat>(chat_model), "Should be Chat model of OpenAI when openai_tool_agent_executor");
            LOG_INFO("Create OpenAIToolAgentExecutor");
            return CreateOpenAIToolAgentExecutor(chat_model, {}, predicate);
        }
    };
}



#endif //LLMOBJECTFACTORY_HPP
