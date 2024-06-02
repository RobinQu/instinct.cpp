//
// Created by RobinQu on 2024/3/15.
//

#ifndef OPENAICOMMONS_HPP
#define OPENAICOMMONS_HPP

#include "LLMGlobals.hpp"
#include "tools/http/HttpUtils.hpp"

namespace
INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    static Endpoint OPENAI_DEFAULT_ENDPOINT{.protocol = kHTTPS, .host = "api.openai.com", .port = 443};
    static const std::string OPENAI_DEFAULT_MODEL_NAME = "gpt-3.5-turbo";

    struct OpenAIConfiguration {
        /**
        * API key for OpenAI service that will be used as bearer token in header
        */
        std::string api_key;

        /**
         * HTTP Endpoint
         */
        Endpoint endpoint = OPENAI_DEFAULT_ENDPOINT;

        /**
         * Modle name
         */
        std::string model_name = OPENAI_DEFAULT_MODEL_NAME;

        /**
         * API Token for OpenAI API
         */
        std::string token;

        std::optional<float> temperature;

        std::optional<float> top_p;

        std::optional<float> seed;

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
