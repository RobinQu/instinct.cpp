//
// Created by RobinQu on 2024/2/13.
//

#ifndef OLLAMAEMBEDDING_H
#define OLLAMAEMBEDDING_H
#include <chrono>

#include <instinct/llm_global.hpp>
#include <instinct/commons/ollama_commons.hpp>
#include <instinct/model/embedding_model.hpp>
#include <instinct/tools/http_rest_client.hpp>
#include <llm.pb.h>

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class OllamaEmbedding final : public IEmbeddingModel {
        HttpRestClient client_;
        OllamaConfiguration configuration_;
        ThreadPool thread_pool_;

    public:
        explicit OllamaEmbedding(const OllamaConfiguration& configuration = {}):
            client_(configuration.endpoint),
            configuration_(configuration), thread_pool_(configuration_.max_parallel) {
        }

        std::vector<Embedding> EmbedDocuments(const std::vector<std::string>& texts) override {
            using namespace std::chrono_literals;
            std::vector<Embedding> result;
            LOG_DEBUG("EmbedDocuments: input.size()={}, configuration_.max_parallel={}", texts.size(), configuration_.max_parallel);

            if (configuration_.max_parallel > 0) {
                const auto batch = client_.CreatePostBatch<OllamaEmbeddingRequest, OllamaEmbeddingResponse>();
                for(const auto& text: texts) {
                    OllamaEmbeddingRequest request;
                    request.set_model(configuration_.model_name);
                    request.set_prompt(text);
                    batch->Add(OLLAMA_EMBEDDING_PATH, request);
                }

                auto futures = batch->Execute(thread_pool_);
                if (configuration_.embedding_timeout_factor > 0s) {
                    // timeout control
                    if (const auto timeout = configuration_.embedding_timeout_factor * texts.size(); !futures.wait_for(timeout)) {
                        throw InstinctException(fmt::format("Embedding request timeout after {} seconds duration", timeout.count()));
                    }
                }
                for(const auto& ollama_resp: futures.get()) {
                    result.emplace_back(ollama_resp.embedding().begin(), ollama_resp.embedding().end());
                }
                return result;
            }

            for (const auto& text: texts) {
                OllamaEmbeddingRequest request;
                request.set_model(configuration_.model_name);
                request.set_prompt(text);
                // TODO handle model options
                // request.mutable_options()->CopyFrom(configuration_->model_options());
                auto response = client_.PostObject<OllamaEmbeddingRequest, OllamaEmbeddingResponse>(
                    OLLAMA_EMBEDDING_PATH, request);
                result.emplace_back(response.embedding().begin(), response.embedding().end());
            }
            return result;
        }

        Embedding EmbedQuery(const std::string& text) override {
            OllamaEmbeddingRequest request;
                request.set_model(configuration_.model_name);
                request.set_prompt(text);
            // TODO handle model options
                // request.mutable_options()->CopyFrom(configuration_->model_options());
            const auto response = client_.PostObject<OllamaEmbeddingRequest, OllamaEmbeddingResponse>(
                OLLAMA_EMBEDDING_PATH, request);
            return {response.embedding().begin(), response.embedding().end()};
        }

        size_t GetDimension() override {
            // Ollama embedding cannot be configured with dimension
            // see https://github.com/ollama/ollama/issues/651
            return configuration_.dimension;
        }
    };

    static void LoadOllamaEmbeddingConfiguration(OllamaConfiguration& configuration) {
        if (StringUtils::IsBlankString(configuration.model_name)) {
            configuration.model_name = SystemUtils::GetEnv("OLLAMA_EMBEDDING_MODEL", OLLAMA_DEFAULT_EMBEDDING_MODEL_NAME);
        }
        if(configuration.dimension == 0) {
            configuration.dimension = SystemUtils::GetIntEnv("OLLAMA_EMBEDDING_DIM");
            if (configuration.dimension == 0) { // guess dimension
                if(configuration.model_name.starts_with("all-minilm")) {
                    configuration.dimension = 384;
                }
                if (configuration.model_name.starts_with("mxbai-embed")) {
                    configuration.dimension = 512;
                }
                if(configuration.model_name.starts_with("nomic-embed-text")) {
                    configuration.dimension = 768;
                }
            }
        }
        if (StringUtils::IsBlankString(configuration.endpoint.host)) {
            configuration.endpoint.host = SystemUtils::GetEnv("OLLAMA_HOST", OLLAMA_ENDPOINT.host);
        }
        if(configuration.endpoint.port == 0) {
            configuration.endpoint.port = SystemUtils::GetIntEnv("OLLAMA_PORT", OLLAMA_ENDPOINT.port);
        }
        if (configuration.endpoint.protocol == kUnspecifiedProtocol) {
            configuration.endpoint.protocol = StringUtils::ToLower(SystemUtils::GetEnv("OLLAMA_PROTOCOL")) == "https" ? kHTTPS : kHTTP;
        }
    }

    static EmbeddingsPtr CreateOllamaEmbedding(OllamaConfiguration ollama_configuration = {}) {
        LoadOllamaEmbeddingConfiguration(ollama_configuration);
        return std::make_shared<OllamaEmbedding>(ollama_configuration);
    }

} // LC_MODEL_NS

#endif //OLLAMAEMBEDDING_H
