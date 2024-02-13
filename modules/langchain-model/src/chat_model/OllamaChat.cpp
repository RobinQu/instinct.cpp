//
// Created by RobinQu on 2024/2/12.
//

#include "OllamaChat.h"

#include "commons/OllamaCommons.h"
#include "message/AIMessage.h"
#include "message/HumanMessage.h"
#include "message/SystemMessage.h"


namespace LC_MODEL_NS {

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
}
