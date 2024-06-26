//
// Created by RobinQu on 2024/1/15.
//

#ifndef OLLAMA_H
#define OLLAMA_H

#include <instinct/tools/HttpRestClient.hpp>
#include <nlohmann/json.hpp>


#include <instinct/llm/BaseLLM.hpp>
#include <instinct/LLMGlobals.hpp>
#include <instinct/commons/OllamaCommons.hpp>


namespace INSTINCT_LLM_NS {


    namespace details {
        static LangaugeModelResult conv_raw_response_to_model_result(const OllamaCompletionResponse& response, bool chunk) {
            LangaugeModelResult result;
            auto* generation = result.add_generations();
            generation->set_is_chunk(chunk);
            generation->set_text(response.response());
            return result;
        }
    }

    class OllamaLLM final : public BaseLLM {
        HttpRestClient http_client_;
        OllamaConfiguration configuration_;
    public:

        explicit OllamaLLM(const OllamaConfiguration& configuration = {}):
                http_client_(configuration.endpoint), configuration_(configuration) {}

        void Configure(const ModelOverrides &options) override {
            if (!options.stop_words.empty()) {
                configuration_.stop_words = options.stop_words;
            }
            if (options.model_name) {
                configuration_.model_name = options.model_name.value();
            }
            if (options.temperature) {
                configuration_.temperature = options.temperature.value();
            }
        }

    private:
        LangaugeModelResult CallOllama(const std::string& prompt) {
            OllamaCompletionRequest request;
            request.set_model(configuration_.model_name);
            request.set_stream(false);
            if (configuration_.json_mode) {
                request.set_format("json");
            }
            request.set_prompt(prompt);
            if (configuration_.seed) {
                request.mutable_options()->set_seed(configuration_.seed.value());
            }
            if (configuration_.temperature) {
                request.mutable_options()->set_temperature(configuration_.temperature.value());
            }
            const auto response = http_client_.PostObject<OllamaCompletionRequest, OllamaCompletionResponse>(OLLAMA_GENERATE_PATH, request);
            return details::conv_raw_response_to_model_result(response, false);
        }

        BatchedLangaugeModelResult Generate(const std::vector<std::string>& prompts) override {
            BatchedLangaugeModelResult result;
            for (const auto& prompt: prompts) {
                auto model_result = CallOllama(prompt);
                result.add_generations()->CopyFrom(model_result);
            }
            return result;
        }

        AsyncIterator<LangaugeModelResult> StreamGenerate(const std::string& prompt) override {
            OllamaCompletionRequest request;
            request.set_prompt(prompt);
            request.set_model(configuration_.model_name);
            request.set_stream(true);
            if (configuration_.json_mode) {
                request.set_format("json");
            }
            if (configuration_.seed) {
                request.mutable_options()->set_seed(configuration_.seed.value());
            }
            if (configuration_.temperature) {
                request.mutable_options()->set_temperature(configuration_.temperature.value());
            }
            if (!configuration_.stop_words.empty()) {
                request.mutable_options()->mutable_stop()->Add(configuration_.stop_words.begin(),
                                                               configuration_.stop_words.end());
            }
            return http_client_.StreamChunkObject<OllamaCompletionRequest, OllamaCompletionResponse>(OLLAMA_GENERATE_PATH, request, true, OLLAMA_SSE_LINE_BREAKER)
                | rpp::operators::map([](const auto& response) {
                    return details::conv_raw_response_to_model_result(response, true);
                });
        }
    };

    static LLMPtr CreateOllamaLLM(OllamaConfiguration configuration = {}) {
        if (StringUtils::IsBlankString(configuration.model_name)) {
            configuration.model_name = SystemUtils::GetEnv("OLLAMA_CHAT_MODEL", OLLAMA_DEFAULT_CHAT_MODEL_NAME);
        }
        if (StringUtils::IsBlankString(configuration.endpoint.host)) {
            configuration.endpoint.host = SystemUtils::GetEnv("OLLAMA_HOST", OLLAMA_ENDPOINT.host);
        }
        if (configuration.endpoint.port == 0) {
            configuration.endpoint.port = SystemUtils::GetIntEnv("OLLAMA_PORT", OLLAMA_ENDPOINT.port);
        }
        if (configuration.endpoint.protocol == kUnspecifiedProtocol) {
            configuration.endpoint.protocol = StringUtils::ToLower(SystemUtils::GetEnv("OLLAMA_PROTOCOL")) == "https" ? kHTTPS : kHTTP;
        }
        return std::make_shared<OllamaLLM>(configuration);
    }


} // model
// langchian

#endif //OLLAMA_H
