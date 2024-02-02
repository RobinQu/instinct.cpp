//
// Created by RobinQu on 2024/1/15.
//

#ifndef OLLAMA_H
#define OLLAMA_H
#include <LangchainCore.h>

#include "tools/HttpRestClient.h"
#include <nlohmann/json.hpp>


namespace langchian::model {
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
        // std::vector<OllamaGenerateMessage> messages;
        std::string prompt;
        std::vector<std::string> images;
        std::string format = "json";
        OllamaGenerateReuqestOption options;
        std::string system;
        // std::string template
        bool stream;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OllamaGenerateRequest, model, prompt, images, format, options, system, stream)


    struct OllamaGenerateResponse {
        std::string model;
        std::string created_at;
        std::vector<unsigned int> context;
        std::string response;
        // std::vector<OllamaGenerateMessage> messages;
        bool done;
        unsigned long total_duration;
        unsigned long load_duration;
        int prompt_eval_count;
        unsigned long  prompt_eval_duration;
        int eval_count;
        unsigned long eval_duration;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OllamaGenerateResponse, model, created_at, context, response, done, total_duration, load_duration, prompt_eval_count, prompt_eval_duration, eval_count, eval_duration)



class OllamaLLM final: public langchain::core::BaseLLM {
    // langchain::core::Endpoint endpoint;
    // std::string endpoint_;
    langchain::core::HttpRestClient http_client_;
public:
    OllamaLLM();
    explicit OllamaLLM(langchain::core::Endpoint endpoint);

protected:
    langchain::core::LLMResultPtr Generate(const std::vector<std::string>& prompts,
        const std::vector<std::string>& stop_words, const langchain::core::OptionDict& options) override;
};

} // model
// langchian

#endif //OLLAMA_H
