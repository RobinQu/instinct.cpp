//
// Created by RobinQu on 2024/1/15.
//

#ifndef OLLAMA_H
#define OLLAMA_H

#include "tools/HttpRestClient.hpp"
#include <nlohmann/json.hpp>


#include "BaseLLM.hpp"
#include "LLMGlobals.hpp"
#include "commons/OllamaCommons.hpp"


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
                BaseLLM(configuration.base_options),
                http_client_(configuration.endpoint), configuration_(configuration) {}
        //
        // std::vector<TokenId> GetTokenIds(const std::string& text) override {
        //
        // }
        //
        // TokenSize GetTokenCount(const std::string& text) override {
        //
        // }
        //
        // TokenSize GetTokenCount(const Message& messages) override {
        //
        // }
        void BindTools(const FunctionToolkitPtr &toolkit) override {
            throw InstinctException("Not supported");
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
            request.mutable_options()->set_seed(configuration_.seed);
            request.mutable_options()->set_temperature(configuration_.temperature);
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
            request.set_model(configuration_.model_name);
            request.set_stream(true);
            if (configuration_.json_mode) {
                request.set_format("json");
            }
            request.mutable_options()->set_seed(configuration_.seed);
            request.mutable_options()->set_temperature(configuration_.temperature);
            request.set_prompt(prompt);
            return http_client_.StreamChunkObject<OllamaCompletionRequest, OllamaCompletionResponse>(OLLAMA_GENERATE_PATH, request)
                | rpp::operators::map([](const auto& response) {
                    return details::conv_raw_response_to_model_result(response, true);
                });
        }
    };



} // model
// langchian

#endif //OLLAMA_H
