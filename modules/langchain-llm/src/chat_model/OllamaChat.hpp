//
// Created by RobinQu on 2024/2/12.
//

#ifndef OLLAMACHAT_H
#define OLLAMACHAT_H
#include "model/BaseChatModel.hpp"
#include "tools/HttpRestClient.hpp"
#include "ModelGlobals.hpp"

LC_LLM_NS {

class OllamaChat: public core::BaseChatModel {
    std::string model_name_;
    core::HttpRestClient client_;
public:
    OllamaChat();
    explicit OllamaChat(core::Endpoint endpoint);
protected:
    core::ChatResultPtr Generate(const std::vector<std::vector<core::BaseMessagePtr>>& messages,
                                 const std::vector<std::string>& stop_words, const core::OptionDict& options) override;
};

    static OllamaGenerateMessage ConvertToOllamaMessage(const core::BaseMessagePtr& m) {
        if(dynamic_pointer_cast<core::HumanMessage>(m)) {
            return {"user", m->GetContent()};
        }
        if(dynamic_pointer_cast<core::AIMessage>(m)) {
            return {"asistant", m->GetContent()};
        }
        if(std::dynamic_pointer_cast<core::SystemMessage>(m)) {
            return {"system", m->GetContent()};
        }
        throw core::LangchainException("unknown message type: " + std::string(typeid(*m).name()));
    }

    OllamaChat::OllamaChat(): BaseChatModel(), client_(OLLAMA_ENDPOINT), model_name_(OLLAMA_DEFUALT_MODEL_NAME) {
    }

    OllamaChat::OllamaChat(core::Endpoint endpoint): BaseChatModel(), client_(std::move(endpoint)), model_name_(OLLAMA_DEFUALT_MODEL_NAME) {
    }

    core::ChatResultPtr OllamaChat::Generate(const std::vector<std::vector<core::BaseMessagePtr>>& messages_batch,
                                             const std::vector<std::string>& stop_words, const core::OptionDict& options) {
        auto chat_result = std::make_shared<core::ChatResult>();
        // TODO better concunrrency control
        for(const auto& messages: messages_batch) {
            auto ollama_messages = messages | std::views::transform([](const core::BaseMessagePtr& message_ptr) {
                return ConvertToOllamaMessage(message_ptr);
            });
            OllamaGenerateRequest request = {
                model_name_,
                {ollama_messages.begin(), ollama_messages.end()},
                {},
                {},
                "json",
                {},
                false
            };
            OllamaGenerateResponse response = client_.PostObject<OllamaGenerateRequest, OllamaGenerateResponse>(OLLAMA_CHAT_PATH, request);
            core::OptionDict option_dict = response;
            chat_result->generations.push_back({
                response.message.content,
                {response.message.content, response.message.role},
                option_dict
            });
        }
        return chat_result;
    }

} // core
// langchain

#endif //OLLAMACHAT_H
