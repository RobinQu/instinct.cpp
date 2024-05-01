//
// Created by RobinQu on 2024/2/13.
//

#ifndef OLLAMACOMMONS_H
#define OLLAMACOMMONS_H

#include <ollama_api.pb.h>

#include "tools/http/HttpUtils.hpp"
#include "LLMGlobals.hpp"

/**
 * @see https://github.com/ollama/ollama/blob/main/docs/api.md
 */
namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;
    using namespace std::chrono_literals;


    static const Endpoint OLLAMA_ENDPOINT {kHTTP, "localhost", 11434};

    static const std::string OLLAMA_GENERATE_PATH = "/api/generate";

    static const std::string OLLAMA_CHAT_PATH = "/api/chat";

    static const std::string OLLAMA_EMBEDDING_PATH = "/api/embeddings";

    static const std::string OLLAMA_DEFUALT_MODEL_NAME = "llama2:latest";


    /**
     * runtime options for ollama. Using struct could in API other than protobuf generated classes makes it easier for users.
     */
    struct OllamaConfiguration {
        ModelOptions base_options = {};
        std::string model_name = OLLAMA_DEFUALT_MODEL_NAME;
        Endpoint endpoint = OLLAMA_ENDPOINT;
        float temperature = 0.8f;
        int seed = 0;
        bool json_mode = false;
        size_t dimension = 4096;
        std::vector<std::string> stop_words = {};

        /**
         * max parallel requests for OpenAI http client
         */
        u_int32_t max_parallel = 0;

        /**
         * Define timeout for generating one embedding
         */
        std::chrono::seconds embedding_timeout_factor = 0s;
    };


}

#endif //OLLAMACOMMONS_H
