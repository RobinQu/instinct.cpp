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

#include "prompt/ChatPromptBuilder.hpp"


namespace INSTINCT_LLM_NS {
    using namespace google::protobuf;
    using namespace INSTINCT_CORE_NS;



    static MessageRoleNameMapping OLLAMA_ROLE_NAME_MAPPING = {
        {kAsistant, "assistant"},
        {kSystem, "system"},
        {kHuman, "user"},
        {kFunction, "asistant"}
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

    class OllamaChat final: public BaseChatModel {
        HttpRestClient client_;
        OllamaConfiguration configuration_;
    public:

        explicit OllamaChat(const OllamaConfiguration& ollama_configuration = {}): client_(ollama_configuration.endpoint) {
        }

        static ChatPromptBuliderPtr CreateChatPromptBuilder() {
            return std::make_shared<ChatPromptBulider>(OLLAMA_ROLE_NAME_MAPPING);
        }

        static ChatPromptTmeplateBuilderPtr CreateChatPromptTemplateBuilder() {
            return std::make_shared<ChatPromptTemplateBulider>(OLLAMA_ROLE_NAME_MAPPING);
        }

        std::vector<TokenId> GetTokenIds(const std::string& text) override {

        }

        TokenSize GetTokenCount(const std::string& text) override {

        }

        TokenSize GetTokenCount(const Message& messages) override {

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
            request.set_format("json");
            request.set_model(configuration_.model_name);
            // request.mutable_options()->CopyFrom(configuration_->model_options());
            auto response = client_.PostObject<OllamaChatCompletionRequest, OllamaChatCompletionResponse>(OLLAMA_CHAT_PATH, request);
            return transform_raw_response(response);
        }

        BatchedLangaugeModelResult Generate(const std::vector<MessageList>& messages) override {
            BatchedLangaugeModelResult batched_langauge_model_result;
            for (const auto& message_list: messages) {
                batched_langauge_model_result.add_generations()->CopyFrom(CallOllama(message_list));
            }
            return batched_langauge_model_result;
        }

        ResultIteratorPtr<LangaugeModelResult> StreamGenerate(const MessageList& message_list) override {
            OllamaChatCompletionRequest request;
            for (const auto& message: message_list.messages()) {
                auto ollama_msg = request.add_messages();
                ollama_msg->set_content(message.content());
                ollama_msg->set_role(message.role());
            }
            request.set_stream(true);
            request.set_format("json");

            request.set_model(configuration_.model_name);
            // request.mutable_options()->CopyFrom(configuration_->model_options());
            auto chunk_itr = client_.StreamChunk<OllamaChatCompletionRequest, OllamaChatCompletionResponse>(OLLAMA_CHAT_PATH, request);

            return create_result_itr_with_transform(transform_raw_response, chunk_itr);
        }


    };


} // core
// langchain

#endif //OLLAMACHAT_H
