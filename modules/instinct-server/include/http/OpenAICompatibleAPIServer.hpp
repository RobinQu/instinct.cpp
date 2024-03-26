//
// Created by RobinQu on 2024/3/26.
//

#ifndef OPENAICOMPATIBLEAPISERVER_HPP
#define OPENAICOMPATIBLEAPISERVER_HPP


#include "HttpLibServer.hpp"
#include "ServerGlobals.hpp"
#include "chain/MessageChain.hpp"
#include "chat_model/OpenAIChat.hpp"
#include "tools/ChronoUtils.hpp"

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

    class OpenAICompatibleAPIServer final: public HttpLibServer {
        using OpenAIChainPtr = MessageChainPtr<OpenAIChatCompletionRequest, OpenAIChatCompletionResponse>;

        OpenAIChainPtr chain_;

    public:
        OpenAICompatibleAPIServer(ServerOptions options, OpenAIChainPtr chain)
            : HttpLibServer(std::move(options)),
              chain_(std::move(chain)) {
            GetHttpLibServer().Post("/v1/chat/completions", [&](const Request& req, Response& resp) {
                long t1 = ChronoUtils::GetCurrentTimeMillis();
                LOG_DEBUG("--> REQ /v1/chat/completions, req={}", req.body);
                const auto openai_req = ProtobufUtils::Deserialize<OpenAIChatCompletionRequest>(req.body);
                const auto openai_response = chain_->Invoke(openai_req);
                if(openai_req.stream()) {
                    auto chunk_response = details::conv_response_to_chunk(openai_response);
                    resp.set_content(ProtobufUtils::Serialize(chunk_response), HTTP_CONTENT_TYPES.at(kJSON));
                } else {
                    resp.set_content(ProtobufUtils::Serialize(openai_response), HTTP_CONTENT_TYPES.at(kJSON));
                }
                LOG_DEBUG("<-- RESP /v1/chat/completions, res={}, rt={}", resp.body, ChronoUtils::GetCurrentTimeMillis()-t1);
            });
        }
    };

}

#endif //OPENAICOMPATIBLEAPISERVER_HPP
