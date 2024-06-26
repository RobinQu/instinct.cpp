//
// Created by RobinQu on 2024/3/15.
//

#ifndef OPENAICOMMONS_HPP
#define OPENAICOMMONS_HPP

#include <instinct/llm_global.hpp>
#include <instinct/tools/http/http_utils.hpp>

namespace
INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    static Endpoint OPENAI_DEFAULT_ENDPOINT{.protocol = kHTTPS, .host = "api.openai.com", .port = 443};
    static const std::string OPENAI_DEFAULT_MODEL_NAME = "gpt-3.5-turbo";
    static const std::string OPENAI_SSE_LINE_BREAKER = "\n\n";

    struct OpenAIConfiguration {
        /**
        * API key for OpenAI service that will be used as bearer token in header
        */
        std::string api_key;

        /**
         * HTTP Endpoint
         */
        Endpoint endpoint {};

        /**
         * Model name
         */
        std::string model_name;

        std::optional<float> temperature;

        std::optional<float> top_p;

        std::optional<int> seed;

        bool json_object = false;

        /**
         * The number of dimensions the resulting output embeddings should have. Only supported in text-embedding-3 and later models.
         */
        int dimension = 0;


        std::optional<int> max_tokens;

        std::vector<std::string> stop_words = {};
    };

    static const std::string DEFAULT_OPENAI_CHAT_COMPLETION_ENDPOINT = "/v1/chat/completions";

    static const std::string DEFAULT_OPENAI_EMBEDDING_ENDPOINT = "/v1/embeddings";
}

#endif //OPENAICOMMONS_HPP
