//
// Created by RobinQu on 2024/5/19.
//

#ifndef LLMOBJECTFACTORY_HPP
#define LLMOBJECTFACTORY_HPP

#include <instinct/llm_global.hpp>
#include <instinct/agent/patterns/llm_compiler/llm_compiler_agent_executor.hpp>
#include <instinct/chat_model/base_chat_model.hpp>
#include <instinct/chat_model/ollama_chat.hpp>
#include <instinct/chat_model/openai_chat.hpp>
#include <instinct/commons/ollama_commons.hpp>
#include <instinct/commons/openai_commons.hpp>
#include <instinct/embedding_model/ollama_embedding.hpp>
#include <instinct/embedding_model/openai_embedding.hpp>

#include "ranker/jina_reranker_model.hpp"
#include "ranker/local_ranking_model.hpp"

namespace INSTINCT_LLM_NS {

    enum ModelProvider {
        kOPENAI = 0,
        kOLLAMA = 1,
        kLOCAL = 2,
        kLLMStudio = 3,
        kLLAMACPP = 4,
        kJINAAI = 5
    };

    static const std::map<std::string, ModelProvider> model_provider_map {
        {"openai", kOPENAI},
        {"ollama", kOLLAMA},
        {"local", kLOCAL},
        {"llm_studio", kLLMStudio},
        {"llama_cpp", kLLAMACPP},
        {"jina_ai", kJINAAI}
    };

    // default values are required
    struct ModelProviderOptions {
        ModelProvider provider;
        std::string model_name;
        // Endpoint endpoint;
        std::string api_key;
        std::string endpoint_url_string;
        int dim = 0;
        OpenAIConfiguration openai;
        OllamaConfiguration ollama;
        JinaConfiguration jina;
    };

    struct AgentExecutorOptions {
        std::string agent_executor_name = "llm_compiler";
        LLMCompilerOptions llm_compiler = {};
    };

    class LLMObjectFactory final {
    public:

        static RankingModelPtr CreateRankingModel(ModelProviderOptions options) {
            switch (options.provider) {
                case kLOCAL: {
                    // only BGE-M3-Reranker is supported right now
                    return CreateLocalRankingModel(BGE_M3_RERANKER);
                }
                case kJINAAI: {
                    options.jina.model_name = options.model_name;
                    options.jina.api_key = options.api_key;
                    if (StringUtils::IsNotBlankString(options.endpoint_url_string)) {
                        const auto req = HttpUtils::CreateRequest("POST " + options.endpoint_url_string);
                        options.jina.endpoint = req.endpoint;
                        options.jina.rerank_path = req.target;
                    }
                    LoadJinaRerankerConfiguration(options.jina);
                    return CreateJinaRerankerModel(options.jina);
                }

                default: {
                    LOG_WARN("Supported provider type for ranking model");
                }
            }
            return nullptr;
        }

        static ChatModelPtr CreateChatModel(ModelProviderOptions options) {
            switch (options.provider) {
                case kOPENAI:
                case kLLMStudio:
                case kLLAMACPP: {
                    options.openai.model_name = options.model_name;
                    options.openai.api_key = options.api_key;
                    if (StringUtils::IsNotBlankString(options.endpoint_url_string)) {
                        const auto req = HttpUtils::CreateRequest("POST " + options.endpoint_url_string);
                        options.openai.endpoint = req.endpoint;
                        options.openai.chat_completion_path = req.target;
                    }
                    LoadOpenAIChatConfiguration(options.openai);
                    return CreateOpenAIChatModel(options.openai);
                }
                case kOLLAMA: {
                    options.ollama.model_name = options.model_name;
                    if (StringUtils::IsNotBlankString(options.endpoint_url_string)) {
                        const auto req = HttpUtils::CreateRequest("POST " + options.endpoint_url_string);
                        options.ollama.chat_completion_path = req.target;
                    }
                    LoadOllamaChatConfiguration(options.ollama);
                    return CreateOllamaChatModel(options.ollama);
                }
                default:
                    return nullptr;
            }
        }

        static EmbeddingsPtr CreateEmbeddingModel(ModelProviderOptions options) {
            switch (options.provider) {
                case kOLLAMA: {
                    options.ollama.model_name = options.model_name;
                    options.ollama.dimension = options.dim;
                    if (StringUtils::IsNotBlankString(options.endpoint_url_string)) {
                        const auto req = HttpUtils::CreateRequest("POST " + options.endpoint_url_string);
                        options.ollama.endpoint = req.endpoint;
                        options.ollama.chat_completion_path = req.target;
                    }
                    LoadOllamaEmbeddingConfiguration(options.ollama);
                    return CreateOllamaEmbedding(options.ollama);
                }
                case kOPENAI:
                case kLLMStudio:
                case kLLAMACPP: {
                    options.openai.model_name = options.model_name;
                    options.openai.api_key = options.api_key;
                    options.openai.dimension = options.dim;
                    if (StringUtils::IsNotBlankString(options.endpoint_url_string)) {
                        const auto req = HttpUtils::CreateRequest("POST " + options.endpoint_url_string);
                        options.openai.endpoint = req.endpoint;
                        options.openai.text_embedding_path = req.target;
                    }
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
