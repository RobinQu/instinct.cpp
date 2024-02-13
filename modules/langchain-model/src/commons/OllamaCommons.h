//
// Created by RobinQu on 2024/2/13.
//

#ifndef OLLAMACOMMONS_H
#define OLLAMACOMMONS_H

#include "CoreGlobals.h"
#include <nlohmann/json.hpp>

namespace LC_MODEL_NS {
    /**
 * https://github.com/ollama/ollama/blob/main/docs/api.md
 */
    struct OllamaGenerateReuqestOption {
        unsigned int temperature;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OllamaGenerateReuqestOption, temperature)

    struct OllamaGenerateMessage {
        std::string role;
        std::string content;
        std::vector<std::string> images;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OllamaGenerateMessage, role, content, images)

    struct OllamaGenerateRequest {
        std::string model = "llama2";
        std::vector<OllamaGenerateMessage> messages;
        std::string prompt;
        std::vector<std::string> images;
        std::string format = "json";
        OllamaGenerateReuqestOption options;
        bool stream;
        // system message template
        // std::string system;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OllamaGenerateRequest, model, prompt, images, format, options, stream)


    struct OllamaGenerateResponse {
        std::string model;
        std::string created_at;
        std::vector<unsigned int> context;
        std::string response;
        OllamaGenerateMessage message;
        bool done;
        unsigned long total_duration;
        unsigned long load_duration;
        int prompt_eval_count;
        unsigned long  prompt_eval_duration;
        int eval_count;
        unsigned long eval_duration;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OllamaGenerateResponse, model, created_at, context, response, done, total_duration, load_duration, prompt_eval_count, prompt_eval_duration, eval_count, eval_duration)

    static const core::Endpoint OLLAMA_ENDPOINT {"localhost", 11434};

    static const std::string OLLAMA_GENERATE_PATH = "/api/generate";

    static const std::string OLLAMA_CHAT_PATH = "/api/chat";

    static const std::string OLLAMA_DEFUALT_MODEL_NAME = "llama2";


}

#endif //OLLAMACOMMONS_H
