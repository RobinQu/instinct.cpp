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

    //
    // struct message_variant_to_ollama_message_converter {
    //     using result_type = OllamaGenerateMessage*;
    //
    //     OllamaChatCompletionRequest* request = nullptr;
    //
    //     result_type operator()(const AIMessage& v) const {
    //         auto msg = request->add_messages();
    //         msg->set_role("assistant");
    //         msg->set_content(v.GetContent);
    //         return msg;
    //     }
    //
    //     result_type operator()(const SystemMessage& v) const {
    //         auto msg = request->add_messages();
    //           msg->set_role("system");
    //           msg->set_content(v.GetContent);
    //           return msg;
    //     }
    //
    //     result_type operator()(const HumanMessage& v) const {
    //         auto msg = request->add_messages();
    //         msg->set_role("user");
    //         msg->set_content(v.GetContent);
    //         return msg;
    //     }
    //
    //     result_type operator()(const FunctionMessage& v) const {
    //         auto msg = request->add_messages();
    //         msg->set_role("assistant");
    //         msg->set_content(v.GetContent);
    //         return msg;
    //     }
    //
    //     result_type operator()(const ChatMessage& v) const {
    //         auto msg = request->add_messages();
    //         msg -> set_role(v.GetRole());
    //         msg -> set_content(v.GetContent);
    //         return msg;
    //     }
    //
    // };


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
        std::shared_ptr<OllamaConfiguration> configuration_;
    public:

        explicit OllamaChat(const std::shared_ptr<OllamaConfiguration>& configuration_): client_(configuration_->endpoint_host(), configuration_->endpoint_port()) {
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
                request.add_messages()->CopyFrom(message);
            }
            request.set_stream(false);
            request.set_format("json");
            request.set_model(configuration_->model_name());
            request.mutable_options()->CopyFrom(configuration_->model_options());
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
                request.add_messages()->CopyFrom(message);
            }
            request.set_stream(true);
            request.set_format("json");
            request.set_model(configuration_->model_name());
            request.mutable_options()->CopyFrom(configuration_->model_options());
            auto chunk_itr = client_.StreamChunk<OllamaChatCompletionRequest, OllamaChatCompletionResponse>(OLLAMA_CHAT_PATH, request);

            return create_result_itr_with_transform(transform_raw_response, chunk_itr);
        }


    };


} // core
// langchain

#endif //OLLAMACHAT_H
