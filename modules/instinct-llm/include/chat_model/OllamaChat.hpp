//
// Created by RobinQu on 2024/2/12.
//

#ifndef OLLAMACHAT_H
#define OLLAMACHAT_H
#include "BaseChatModel.hpp"
#include "tools/HttpRestClient.hpp"
#include "LLMGlobals.hpp"
#include "commons/OllamaCommons.hpp"
#include <llm.pb.h>


namespace INSTINCT_LLM_NS {
    using namespace google::protobuf;
    using namespace INSTINCT_CORE_NS;



    static MessageRoleNameMapping OLLAMA_ROLE_NAME_MAPPING = {
        {kAsisstant, "assistant"},
        {kSystem,    "system"},
        {kHuman,     "user"},
        {kFunction,  "asistant"}
    };

    static LangaugeModelResult transform_raw_response(const OllamaChatCompletionResponse& response) {
        LangaugeModelResult model_result;
        auto* generation = model_result.add_generations();
        generation->set_is_chunk(false);
        auto *generation_msg = generation->mutable_message();
        generation_msg->set_content(response.message().content());
        generation_msg->set_role(response.message().role());
        generation->set_text(fmt::format("{}: {}", generation_msg->role(), generation->text()));
        return model_result;
    }

    class OllamaChat final: public BaseChatModel, public virtual IConfigurable<OllamaConfiguration> {
        HttpRestClient client_;
        OllamaConfiguration configuration_;
    public:
        explicit OllamaChat(const OllamaConfiguration& ollama_configuration = {}):
                client_(ollama_configuration.endpoint),
                configuration_(ollama_configuration) {
        }

        void Configure(const OllamaConfiguration &options) override {
            configuration_ = options;
        }

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
        LangaugeModelResult CallOllama(const MessageList& message_list) {
            OllamaChatCompletionRequest request;
            for (const auto& message: message_list.messages()) {
                auto ollama_msg = request.add_messages();
                ollama_msg->set_content(message.content());
                ollama_msg->set_role(message.role());
            }
            request.set_stream(false);
            if (configuration_.json_mode) {
                request.set_format("json");
            }
            request.set_model(configuration_.model_name);
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
            const auto response = client_.PostObject<OllamaChatCompletionRequest, OllamaChatCompletionResponse>(OLLAMA_CHAT_PATH, request);
            return transform_raw_response(response);
        }

        BatchedLangaugeModelResult Generate(const std::vector<MessageList>& messages) override {
            BatchedLangaugeModelResult batched_language_model_result;
            for (const auto& message_list: messages) {
                batched_language_model_result.add_generations()->CopyFrom(CallOllama(message_list));
            }
            return batched_language_model_result;
        }

        AsyncIterator<LangaugeModelResult> StreamGenerate(const MessageList& message_list) override {
            OllamaChatCompletionRequest request;
            for (const auto& message: message_list.messages()) {
                auto ollama_msg = request.add_messages();
                ollama_msg->set_content(message.content());
                ollama_msg->set_role(message.role());
            }
            request.set_stream(true);
            if (configuration_.json_mode) {
                request.set_format("json");
            }
            request.set_model(configuration_.model_name);
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

            return  client_.StreamChunkObject<OllamaChatCompletionRequest, OllamaChatCompletionResponse>(OLLAMA_CHAT_PATH, request, true)
                | rpp::operators::map(transform_raw_response);
        }

    };

    static void LoadOllamaChatConfiguration(OllamaConfiguration& configuration) {
        if (StringUtils::IsBlankString(configuration.model_name)) {
            configuration.model_name = SystemUtils::GetEnv("OLLAMA_CHAT_MODEL", OLLAMA_DEFAULT_CHAT_MODEL_NAME);
        }
        if (StringUtils::IsBlankString(configuration.endpoint.host)) {
            configuration.endpoint.host = SystemUtils::GetEnv("OLLAMA_HOST", OLLAMA_ENDPOINT.host);
        }
        if (configuration.endpoint.port == 80) {
            configuration.endpoint.port = SystemUtils::GetIntEnv("OLLAMA_PORT", OLLAMA_ENDPOINT.port);
        }
        if (configuration.endpoint.protocol == kHTTP) {
            configuration.endpoint.protocol = StringUtils::ToLower(SystemUtils::GetEnv("OLLAMA_PROTOCOL")) == "https" ? kHTTPS : kHTTP;
        }
    }

    static ChatModelPtr CreateOllamaChatModel(OllamaConfiguration configuration = {}) {
        LoadOllamaChatConfiguration(configuration);
        return std::make_shared<OllamaChat>(configuration);
    }


} // core


#endif //OLLAMACHAT_H
