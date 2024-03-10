//
// Created by RobinQu on 2024/1/15.
//

#ifndef OLLAMA_H
#define OLLAMA_H

#include "tools/HttpRestClient.hpp"
#include <nlohmann/json.hpp>

#include "CoreTypes.hpp"
#include "BaseLLM.hpp"
#include "LLMGlobals.hpp"
#include "commons/OllamaCommons.hpp"


namespace INSTINCT_LLM_NS {


    namespace details {
        static LangaugeModelResult conv_raw_response_to_model_result(const OllamaCompletionResponse& response, bool chunk) {
            LangaugeModelResult result;
            auto* genertion = result.add_generations();
            genertion->set_is_chunk(chunk);
            genertion->set_text(response.response());
            return result;
        }
    }

    class OllamaLLM final : public BaseLLM {
        HttpRestClient http_client_;
        OllamaConfiguration configuration_;
    public:

        explicit OllamaLLM(const OllamaConfiguration& configuration = {}): http_client_(configuration.endpoint), configuration_(configuration) {}

        std::vector<TokenId> GetTokenIds(const std::string& text) override {

        }

        TokenSize GetTokenCount(const std::string& text) override {

        }

        TokenSize GetTokenCount(const Message& messages) override {

        }

    private:
        LangaugeModelResult CallOllama(const std::string& prompt) {
            OllamaCompletionRequest request;
            request.set_model(configuration_.model_name);
            request.set_stream(false);
            request.set_format("json");
            request.set_prompt(prompt);
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

        ResultIteratorPtr<LangaugeModelResult> StreamGenerate(const std::string& prompt) override {
            OllamaCompletionRequest request;
            request.set_model(configuration_.model_name);
            request.set_stream(true);
            request.set_format("json");
            request.set_prompt(prompt);
            auto chunk_itr = http_client_.StreamChunk<OllamaCompletionRequest, OllamaCompletionResponse>(OLLAMA_GENERATE_PATH, request);
            return create_result_itr_with_transform([](auto&& response) -> LangaugeModelResult {
                // std::cout << "\"" << response.response() << "\"" << std::endl;
                return details::conv_raw_response_to_model_result(response, true);
            }, chunk_itr);
        }
    };



} // model
// langchian

#endif //OLLAMA_H
