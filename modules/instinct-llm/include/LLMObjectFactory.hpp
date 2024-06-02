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
#include "embedding_model/OllamaEmbedding.hpp"
#include "embedding_model/OpenAIEmbedding.hpp"

namespace INSTINCT_LLM_NS {

    // default values are required
    struct LLMProviderOptions {
        std::string provider_name = "openai";
        std::string chat_model_name;
        std::string embedding_model_name;
        OpenAIConfiguration openai = {};
        OllamaConfiguration ollama = {};
    };

    struct AgentExecutorOptions {
        std::string agent_executor_name = "llm_compiler";
        LLMCompilerOptions llm_compiler = {};
    };


    class LLMObjectFactory final {
    public:

        static ChatModelPtr CreateChatModel(LLMProviderOptions options) {
            if (options.provider_name == "ollama") {
                LoadOllamaChatConfiguration(options.ollama);
                return CreateOllamaChatModel(options.ollama);
            }
            if (options.provider_name == "openai") {
                LoadOpenAIChatConfiguration(options.openai);
                return CreateOpenAIChatModel(options.openai);
            }
            return nullptr;
        }

        static EmbeddingsPtr CreateEmbeddingModel(LLMProviderOptions options) {
            if (options.provider_name == "ollama") {
                LoadOllamaEmbeddingConfiguration(options.ollama);
                return CreateOllamaEmbedding(options.ollama);
            }
            if (options.provider_name == "openai") {
                LoadOpenAIEmbeddingConfiguration(options.openai);
                return CreateOpenAIEmbeddingModel(options.openai);
            }
            return nullptr;
        }

        static AgentExecutorPtr CreateAgentExecutor(
            const AgentExecutorOptions& options,
            const ChatModelPtr& chat_model,
            const StopPredicate& predicate,
            const std::vector<FunctionToolkitPtr>& tk = {}
        ) {
            if(options.agent_executor_name == "llm_compiler") {
                LOG_INFO("Create LLMCompilerAgentExecutor");
                return CreateLLMCompilerAgentExecutor(chat_model, tk, predicate, options.llm_compiler);
            }
            assert_true(std::dynamic_pointer_cast<OpenAIChat>(chat_model), "Should be Chat model of OpenAI when openai_tool_agent_executor");
            LOG_INFO("Create OpenAIToolAgentExecutor");
            return CreateOpenAIToolAgentExecutor(chat_model, tk, predicate);
        }
    };
}



#endif //LLMOBJECTFACTORY_HPP
