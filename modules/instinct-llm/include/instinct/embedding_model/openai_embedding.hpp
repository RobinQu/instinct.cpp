//
// Created by RobinQu on 2024/3/15.
//

#ifndef OPENAIEMBEDDING_HPP
#define OPENAIEMBEDDING_HPP

#include <instinct/embedding_api.pb.h>
#include <instinct/model/embedding_model.hpp>

#include <instinct/llm_global.hpp>
#include <instinct/commons/openai_commons.hpp>
#include <instinct/tools/http_rest_client.hpp>

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class OpenAIEmbedding final: public IEmbeddingModel {
        OpenAIConfiguration configuration_;
        HttpRestClient client_;

    public:
        explicit OpenAIEmbedding(OpenAIConfiguration configuration)
            : configuration_(std::move(configuration)), client_(*configuration_.endpoint) {
            assert_gt(configuration.dimension, 0, "dimension should be greater than zero");
            client_.GetDefaultHeaders().emplace("Authorization", fmt::format("Bearer {}", configuration_.api_key));
        }

        std::vector<Embedding> EmbedDocuments(const std::vector<std::string>& texts) override {
            assert_lte(texts.size(), 2048, "Number of given texts should be less than 2048");
            std::vector<Embedding> result;
            result.reserve(texts.size());

            OpenAIEmbeddingRequest req;
            for(const auto& text: texts) {
                req.add_input(text);
            }
            req.set_model(configuration_.model_name);
            // req.set_dimension(configuration_.dimension);
            const auto res = client_.PostObject<OpenAIEmbeddingRequest, OpenAIEmbeddingResponse>(configuration_.text_embedding_path, req);

            assert_true(res.data_size()>0, "should have at least one embedding returned");
            for(const auto&embedding_response: res.data()) {
                Embedding embedding;
                embedding.reserve(configuration_.dimension);
                for (const auto& f: embedding_response.embedding()) {
                    embedding.push_back(f);
                }
                result.push_back(embedding);
            }
            return result;
        }

        Embedding EmbedQuery(const std::string& text) override {
            OpenAIEmbeddingRequest req;
            req.add_input(text);
            req.set_model(configuration_.model_name);
            // req.set_dimension(configuration_.dimension);
            auto res = client_.PostObject<OpenAIEmbeddingRequest, OpenAIEmbeddingResponse>(configuration_.text_embedding_path, req);
            Embedding embedding;
            embedding.reserve(configuration_.dimension);
            assert_true(res.data_size()>0, "should have at least one embedding returned");
            for(const auto&f: res.data(0).embedding()) {
                embedding.push_back(f);
            }
            return embedding;
        }

        size_t GetDimension() override {
            return configuration_.dimension;
        }
    };

    static void LoadOpenAIEmbeddingConfiguration(OpenAIConfiguration& configuration) {
        if(StringUtils::IsBlankString(configuration.api_key)) {
            configuration.api_key = SystemUtils::GetEnv("OPENAI_API_KEY");
        }
        if (StringUtils::IsBlankString(configuration.model_name)) {
            configuration.model_name = SystemUtils::GetEnv("OPENAI_EMBEDDING_MODEL", "text-embedding-3-small");
        }
        if (configuration.dimension == 0) {
            configuration.dimension = SystemUtils::GetIntEnv("OPENAI_EMBEDDING_DIM");
            if (configuration.dimension == 0) { // guess dimension
                // https://platform.openai.com/docs/guides/embeddings/what-are-embeddings
                if (configuration.model_name == "text-embedding-3-large") configuration.dimension = 3072;
                if (configuration.model_name == "text-embedding-3-small") configuration.dimension = 1536;
                if (configuration.model_name == "text-embedding-ada-002") configuration.dimension = 1536;
            }
        }
        if (!configuration.endpoint) {
            const auto endpoint_url_env = SystemUtils::GetEnv("OPENAI_EMBEDDING_API_ENDPOINT");
            if (StringUtils::IsBlankString(endpoint_url_env)) {
                configuration.endpoint = OPENAI_DEFAULT_ENDPOINT;
            } else {
                const auto req = HttpUtils::CreateRequest("POST " + endpoint_url_env);
                configuration.endpoint = req.endpoint;
                configuration.text_embedding_path = req.target;
            }
        }
        if (StringUtils::IsBlankString(configuration.text_embedding_path)) {
            configuration.text_embedding_path = DEFAULT_OPENAI_EMBEDDING_ENDPOINT;
        }
    }

    static EmbeddingsPtr CreateOpenAIEmbeddingModel(OpenAIConfiguration configuration = {}) {
        LoadOpenAIEmbeddingConfiguration(configuration);
        return std::make_shared<OpenAIEmbedding>(configuration);
    }

}

#endif //OPENAIEMBEDDING_HPP
