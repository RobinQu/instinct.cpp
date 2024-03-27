//
// Created by RobinQu on 2024/3/15.
//

#ifndef OPENAICOMMONS_HPP
#define OPENAICOMMONS_HPP

#include "LLMGlobals.hpp"
#include "tools/http/HttpUtils.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    static Endpoint OPENAI_DEFAULT_ENDPOINT {.protocol = kHTTPS, .host = "api.openai.com", .port = 443};

    struct  OpenAIConfiguration {
        /**
        * API key for OpenAI service that will be used as bearer token in header
        */
        std::string api_key;

        /**
         * options for base class
         */
        ModelOptions base_options = {};

        /**
         * HTTP Endpoint
         */
        Endpoint endpoint = OPENAI_DEFAULT_ENDPOINT;

        /**
         * Modle name
         */
        std::string model_name;

        /**
         * API Token for OpenAI API
         */
        std::string token;

        float temperature;

        int seed;

        bool json_object;

        /**
         * only used in embedding API
         */
        size_t dimension;
    };

    static const std::string DEFAULT_OPENAI_CHAT_COMPLETION_ENDPOINT = "/v1/chat/completions";

    static const std::string DEFAULT_OPENAI_EMBEDDING_ENDPOINT = "/v1/embeddings";
}

#endif //OPENAICOMMONS_HPP
