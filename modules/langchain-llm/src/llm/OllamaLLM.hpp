//
// Created by RobinQu on 2024/1/15.
//

#ifndef OLLAMA_H
#define OLLAMA_H

#include "tools/HttpRestClient.hpp"
#include <nlohmann/json.hpp>

#include "CoreTypes.hpp"
#include "model/BaseLLM.hpp"
#include "model/LLMResult.hpp"
#include "ModelGlobals.hpp"
#include "commons/OllamaCommons.hpp"


LC_LLM_NS {


    class OllamaLLM final : public langchain::core::BaseLLM {
        core::HttpRestClient http_client_;
    public:
        OllamaLLM();

        explicit OllamaLLM(langchain::core::Endpoint endpoint);


        std::vector<core::TokenId> GetTokenIds(const std::string& text) override;

        core::TokenSize GetTokenCount(const std::string& text) override;

        core::TokenSize GetTokenCount(const core::MessageVariants& messages) override;

    protected:
        core::LLMResult Generate(const std::vector<std::string>& prompts,
            const core::LLMRuntimeOptions& runtime_options) override;
    };

    inline OllamaLLM::OllamaLLM(): langchain::core::BaseLLM(), http_client_(OLLAMA_ENDPOINT) {
    }

    inline OllamaLLM::OllamaLLM(langchain::core::Endpoint endpoint): BaseLLM(), http_client_(std::move(endpoint)) {
    }


    inline std::vector<core::TokenId> OllamaLLM::GetTokenIds(const std::string& text) {
        return {};
    }

    inline core::TokenSize OllamaLLM::GetTokenCount(const std::string& text) {
        return 0;
    }

    inline core::TokenSize OllamaLLM::GetTokenCount(const core::MessageVariants& messages) {
        return 0;
    }

    inline core::LLMResult OllamaLLM::Generate(const std::vector<std::string>& prompts,
        const core::LLMRuntimeOptions& runtime_options) {
        auto result = core::LLMResult();
        for (const auto& prompt: prompts) {
            OllamaGenerateRequest request {
                runtime_options.model_name,
                prompt,
                {},
                "json",
                {},
                false
            };

            const auto ollama_response = http_client_.PostObject<OllamaGenerateRequest, OllamaGenerateResponse>(
                OLLAMA_GENERATE_PATH, request);
            core::OptionDict option_dict = ollama_response;
            std::vector<core::GenerationVariant> generation;

            std::cout << "model response: " << ollama_response.response << std::endl;

            generation.emplace_back(core::Generation{ollama_response.response, option_dict, "LLMGeneration"});
            result.generations.emplace_back(generation);
        }
        return result;
    }

} // model
// langchian

#endif //OLLAMA_H
