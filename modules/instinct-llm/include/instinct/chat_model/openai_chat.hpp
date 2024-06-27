//
// Created by RobinQu on 2024/3/15.
//

#ifndef OPENAICHAT_HPP
#define OPENAICHAT_HPP

#include <instinct/llm.pb.h>

#include <instinct/chat_model/base_chat_model.hpp>
#include <instinct/llm_global.hpp>
#include <instinct/commons/openai_commons.hpp>
#include <instinct/tools/http_rest_client.hpp>


namespace INSTINCT_LLM_NS {
    /**
    * OpenAI API endpoint reference:
    * https://platform.openai.com/docs/api-reference/chat/create
    */
    class OpenAIChat final: public BaseChatModel {
        OpenAIConfiguration configuration_;
        HttpRestClient client_;
        std::vector<OpenAIChatCompletionRequest_ChatCompletionTool> function_tools_;
    public:
        explicit OpenAIChat(OpenAIConfiguration configuration)
            :  configuration_(std::move(configuration)), client_(configuration_.endpoint) {
            client_.GetDefaultHeaders().emplace("Authorization", fmt::format("Bearer {}", configuration_.api_key));
        }

        void BindToolSchemas(const std::vector<FunctionTool> &function_tool_schema) override {
            function_tools_.clear();
            for(const auto& function_tool: function_tool_schema) {
                OpenAIChatCompletionRequest_ChatCompletionTool tool;
                tool.set_type("function");
                tool.mutable_function()->CopyFrom(function_tool);
                function_tools_.push_back(tool);
            }
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
            if (options.top_p) {
                configuration_.top_p = options.top_p.value();
            }
        }

        void CallOpenAI(const MessageList& message_list, BatchedLangaugeModelResult& batched_language_model_result) {
            const auto req = BuildRequest_(message_list, false);
            const auto resp = client_.PostObject<OpenAIChatCompletionRequest, OpenAIChatCompletionResponse>(DEFAULT_OPENAI_CHAT_COMPLETION_ENDPOINT, req);

            auto* language_model_result = batched_language_model_result.add_generations();
            for(const auto& choice: resp.choices()) {
                auto* single_result = language_model_result->add_generations();
                single_result->set_text(choice.message().content());
                single_result->set_is_chunk(false);
                single_result->mutable_message()->CopyFrom(choice.message());

                // unescape common characters
                // https://www.reddit.com/r/LocalLLaMA/comments/1agrddy/has_anyone_encountered_mistrals_tendency_to_use/
                std::string unescaped = std::regex_replace(choice.message().content(), std::regex {R"(\\_)"}, "_");
                single_result->mutable_message()->set_content(unescaped);
            }
        }

        BatchedLangaugeModelResult Generate(const std::vector<MessageList>& message_matrix) override {
            // TODO make it parallel
            BatchedLangaugeModelResult batched_language_model_result;
            for (const auto& message_list: message_matrix) {
                CallOpenAI(message_list, batched_language_model_result);
            }
            return batched_language_model_result;
        }

        AsyncIterator<LangaugeModelResult> StreamGenerate(const MessageList& messages) override {
            const auto req = BuildRequest_(messages, true);
            const auto chunk_itr = client_.StreamChunkObject<OpenAIChatCompletionRequest, OpenAIChatCompletionChunk>(DEFAULT_OPENAI_CHAT_COMPLETION_ENDPOINT, req, true, OPENAI_SSE_LINE_BREAKER, {"[DONE]"});
            return chunk_itr | rpp::operators::map([](const OpenAIChatCompletionChunk& chunk) {
                LangaugeModelResult language_model_result;
                for (const auto& choice: chunk.choices()) {
                    auto* single_result = language_model_result.add_generations();
                    single_result->set_text(choice.delta().content());
                    single_result->set_is_chunk(true);
                    single_result->mutable_message()->CopyFrom(choice.delta());
                }
                return language_model_result;
            });
        }

    private:
        OpenAIChatCompletionRequest BuildRequest_(const MessageList& message_list, const bool stream) {
            OpenAIChatCompletionRequest req;
            for (const auto& msg: message_list.messages()) {
                LOG_DEBUG("msg=[{}],role={},tool_calls_size={}", msg.content(), msg.role(), msg.tool_calls_size());
                req.add_messages()->CopyFrom(msg);
            }
            req.set_model(configuration_.model_name);
            req.set_n(1);
            if (configuration_.seed) {
                req.set_seed(configuration_.seed.value());
            }

            if (configuration_.temperature) {
                req.set_temperature(configuration_.temperature.value());
            }
            if (configuration_.top_p) {
                req.set_top_p(configuration_.top_p.value());
            }
            if (configuration_.max_tokens) {
                req.set_max_tokens(configuration_.max_tokens.value());
            }
            if (configuration_.json_object) {
                req.mutable_response_format()->set_type("json_object");
            }
            req.mutable_tools()->Add(function_tools_.begin(), function_tools_.end());
            req.set_stream(stream);
            req.mutable_stop()->Add(configuration_.stop_words.begin(), configuration_.stop_words.end());
            return req;
        }

    };



    static void LoadOpenAIChatConfiguration(OpenAIConfiguration& configuration) {
        if (StringUtils::IsBlankString(configuration.api_key)) {
            configuration.api_key = SystemUtils::GetEnv("OPENAI_API_KEY");
        }
        if(StringUtils::IsBlankString(configuration.model_name)) {
            configuration.model_name = SystemUtils::GetEnv("OPENAI_CHAT_MODEL", "gpt-3.5-turbo");
        }
        if (StringUtils::IsBlankString(configuration.endpoint.host)) {
            configuration.endpoint.host = SystemUtils::GetEnv("OPENAI_HOST", OPENAI_DEFAULT_ENDPOINT.host);
        }
        if (configuration.endpoint.port == 0) {
            configuration.endpoint.port = SystemUtils::GetIntEnv("OPENAI_PORT", OPENAI_DEFAULT_ENDPOINT.port);
        }
        if (configuration.endpoint.protocol == kUnspecifiedProtocol) {
            configuration.endpoint.protocol = StringUtils::ToLower(SystemUtils::GetEnv("OPENAI_PROTOCOL", "https")) == "https" ? kHTTPS : kHTTP;
        }
    }


    static ChatModelPtr CreateOpenAIChatModel(OpenAIConfiguration configuration = {}) {
        LoadOpenAIChatConfiguration(configuration);
        return std::make_shared<OpenAIChat>(configuration);
    }

}


#endif //OPENAICHAT_HPP
