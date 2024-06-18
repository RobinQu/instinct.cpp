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

    enum ModelProvider {
        kOPENAI = 0,
        kOLLAMA = 1,
        kLOCAL = 2,
        kLLMStudio = 3,
        kLLAMACPP = 4
    };

    static const std::map<std::string, ModelProvider> model_provider_map {
        {"openai", kOPENAI},
        {"ollama", kOLLAMA},
        {"local", kLOCAL},
        {"llm_studio", kLLMStudio},
        {"llama_cpp", kLLAMACPP},
    };


    // default values are required
    struct LLMProviderOptions {
        ModelProvider provider;
        std::string model_name;
        Endpoint endpoint;
        std::string api_key;
        int dim = 0;
        OpenAIConfiguration openai;
        OllamaConfiguration ollama;
    };

    struct AgentExecutorOptions {
        std::string agent_executor_name = "llm_compiler";
        LLMCompilerOptions llm_compiler = {};
    };

    class LLMObjectFactory final {
    public:

        static ChatModelPtr CreateChatModel(LLMProviderOptions options) {
            switch (options.provider) {
                case kOPENAI:
                case kLLMStudio:
                case kLLAMACPP: {
                    options.openai.model_name = options.model_name;
                    options.openai.endpoint = options.endpoint;
                    options.openai.api_key = options.api_key;
                    LoadOpenAIChatConfiguration(options.openai);
                    return CreateOpenAIChatModel(options.openai);
                }
                case kOLLAMA: {
                    options.ollama.model_name = options.model_name;
                    options.ollama.endpoint = options.endpoint;
                    LoadOllamaChatConfiguration(options.ollama);
                    return CreateOllamaChatModel(options.ollama);
                }
                default:
                    return nullptr;
            }
        }

        static EmbeddingsPtr CreateEmbeddingModel(LLMProviderOptions options) {
            switch (options.provider) {
                case kOLLAMA: {
                    options.ollama.model_name = options.model_name;
                    options.ollama.endpoint = options.endpoint;
                    options.ollama.dimension = options.dim;
                    LoadOllamaEmbeddingConfiguration(options.ollama);
                    return CreateOllamaEmbedding(options.ollama);
                }
                case kOPENAI:
                case kLLMStudio:
                case kLLAMACPP: {
                    options.openai.model_name = options.model_name;
                    options.openai.endpoint = options.endpoint;
                    options.openai.api_key = options.api_key;
                    options.openai.dimension = options.dim;
                    LoadOpenAIEmbeddingConfiguration(options.openai);
                    return CreateOpenAIEmbeddingModel(options.openai);
                }
                default:
                    return nullptr;
            }
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
