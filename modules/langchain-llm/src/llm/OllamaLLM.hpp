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

        std::vector<std::string> Stream(
            const std::variant<core::StringPromptValue, core::ChatPromptValue, std::string, std::vector<std::variant<
            core::AIMessage, core::HumanMessage, core::FunctionMessage, core::SystemMessage, core::ChatMessage>>>&
            input, const core::LLMRuntimeOptions& options) override;

        std::vector<core::TokenId> GetTokenIds(const std::string& text) override;

        core::TokenSize GetTokenCount(const std::string& text) override;

        core::TokenSize GetTokenCount(const std::vector<core::BaseMessagePtr>& messages) override;

    protected:
        core::LLMResult Generate(const std::vector<std::string>& prompts,
            const core::LLMRuntimeOptions& runtime_options) override;
    };

    inline OllamaLLM::OllamaLLM(): langchain::core::BaseLLM(), http_client_(OLLAMA_ENDPOINT) {
    }

    inline OllamaLLM::OllamaLLM(langchain::core::Endpoint endpoint): BaseLLM(), http_client_(std::move(endpoint)) {
    }

    inline std::vector<std::string> OllamaLLM::Stream(
        const std::variant<core::StringPromptValue, core::ChatPromptValue, std::string, std::vector<std::variant<core::
        AIMessage, core::HumanMessage, core::FunctionMessage, core::SystemMessage, core::ChatMessage>>>& input,
        const core::LLMRuntimeOptions& options) {
        return {};
    }

    inline std::vector<core::TokenId> OllamaLLM::GetTokenIds(const std::string& text) {
        return {};
    }

    inline core::TokenSize OllamaLLM::GetTokenCount(const std::string& text) {
        return 0;
    }

    inline core::TokenSize OllamaLLM::GetTokenCount(const std::vector<core::BaseMessagePtr>& messages) {
        return 0;
    }

    inline core::LLMResult OllamaLLM::Generate(const std::vector<std::string>& prompts,
        const core::LLMRuntimeOptions& runtime_options) {
        auto result = core::LLMResult();
        for (const auto& prompt: prompts) {
            OllamaGenerateRequest request;
            request.prompt = prompt;
            auto ollama_response = http_client_.PostObject<OllamaGenerateRequest, OllamaGenerateResponse>(
                OLLAMA_GENERATE_PATH, request);
            core::OptionDict option_dict = ollama_response;
            std::vector<core::GenerationVariant> generation;
            generation.emplace_back(core::Generation{ollama_response.response, option_dict, "LLMGeneration"});
            result.generations.emplace_back(generation);
        }
        return result;
    }

} // model
// langchian

#endif //OLLAMA_H
