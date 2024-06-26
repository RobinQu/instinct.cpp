//
// Created by RobinQu on 2024/2/13.
//

#ifndef OLLAMACOMMONS_H
#define OLLAMACOMMONS_H

#include <ollama_api.pb.h>

#include <instinct/tools/http/http_utils.hpp>
#include <instinct/LLMGlobals.hpp>

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

    static const std::string OLLAMA_DEFAULT_CHAT_MODEL_NAME = "mistral:latest";

    static const std::string OLLAMA_DEFAULT_EMBEDDING_MODEL_NAME = "all-minilm:latest";

    static const std::string OLLAMA_SSE_LINE_BREAKER = "\n";

    /**
     * runtime options for ollama. Using struct could in API other than protobuf generated classes makes it easier for users.
     */
    struct OllamaConfiguration {
        std::string model_name;
        Endpoint endpoint {};
        std::optional<float> temperature;
        std::optional<int> seed;
        bool json_mode = false;
        size_t dimension = 0;
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
