//
// Created by RobinQu on 2024/3/26.
//

#ifndef OPENAICOMPATIBLEAPISERVER_HPP
#define OPENAICOMPATIBLEAPISERVER_HPP


#include <utility>

#include <instinct/server_global.hpp>

#include <instinct/chain/message_chain.hpp>
#include <instinct/chat_model/openai_chat.hpp>
#include <instinct/endpoint/chat_completion/chat_completion_request_input_parser.hpp>
#include <instinct/endpoint/chat_completion/chat_completion_response_output_parser.hpp>
#include <instinct/server/HttpController.hpp>
#include <instinct/server/httplib/http_lib_server.hpp>
#include <instinct/tools/chrono_utils.hpp>

namespace INSTINCT_SERVER_NS {
    using namespace INSTINCT_LLM_NS;

    namespace details {
        static OpenAIChatCompletionChunk conv_response_to_chunk(const OpenAIChatCompletionResponse& resp) {
            OpenAIChatCompletionChunk chunk;
            chunk.set_id(resp.id());
            for(const auto& choice: resp.choices()) {
                auto* chunk_choice = chunk.add_choices();
                chunk_choice->set_index(choice.index());
                chunk_choice->mutable_logprobs()->CopyFrom(choice.logprobs());
                chunk_choice->set_finish_reason(choice.finish_reason());
                auto* delta = chunk_choice->mutable_delta();
                delta->set_name(choice.message().name());
                delta->set_content(choice.message().content());
                delta->set_role(choice.message().role());
            }
            chunk.set_created(resp.created());
            chunk.set_model(resp.model());
            chunk.set_system_fingerprint(resp.system_fingerprint());
            chunk.set_object("chat.completion.chunk");
            return chunk;
        }
    }

    using OpenAIChainPtr = MessageChainPtr<OpenAIChatCompletionRequest,OpenAIChatCompletionResponse>;

    class ChatCompletionController final: public HttpLibController {
        OpenAIChainPtr chain_;
    public:
        explicit ChatCompletionController(OpenAIChainPtr chain)
            : chain_(std::move(chain)) {
        }

        void OnServerCreated(HttpLibServer &server) override {
            server.GetHttpLibServer().set_exception_handler([](const auto& req, auto& res, std::exception_ptr ep) {
                JSONObject error_response;
                try {
                    std::rethrow_exception(ep);
                } catch (const std::exception& ex) {
                    LOG_ERROR("Failed request in ChatCompletionController. ex.what={}", ex.what());
                    error_response["message"] = ex.what();
                } catch (...) {
                    error_response["message"] = "unknown exception occurred.";
                }
                res.set_content(error_response.dump(), HTTP_CONTENT_TYPES.at(kJSON));
            });
        }

        void Mount(HttpLibServer &server) override {
            server.GetHttpLibServer().Post("/v1/chat/completions", [&](const Request& req, Response& resp) {
                log_guard log {req.method, req.path};
                const auto openai_req = ProtobufUtils::Deserialize<OpenAIChatCompletionRequest>(req.body);
                const auto openai_response = chain_->Invoke(openai_req);
                if(openai_req.stream()) {
                    auto chunk_response = details::conv_response_to_chunk(openai_response);
                    resp.set_content(ProtobufUtils::Serialize(chunk_response), HTTP_CONTENT_TYPES.at(kJSON));
                } else {
                    resp.set_content(ProtobufUtils::Serialize(openai_response), HTTP_CONTENT_TYPES.at(kJSON));
                }
            });
        }
    };

    static HttpLibControllerPtr CreateOpenAIChatCompletionController(
        const StepFunctionPtr& fn,
        const OpenAIChatCompletionInputParserOptions& input_parser_options = {}
    ) {
        const auto chain = CreateFunctionalChain<OpenAIChatCompletionRequest, OpenAIChatCompletionResponse>(
            std::make_shared<ChatCompletionRequestInputParser>(input_parser_options),
            std::make_shared<ChatCompletionResponseOutputParser>(),
            fn
        );
        return std::make_shared<ChatCompletionController>(chain);
    }

}

#endif //OPENAICOMPATIBLEAPISERVER_HPP
