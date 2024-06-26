//
// Created by RobinQu on 2024/3/26.
//

#ifndef OPENAICHATCOMPLETIONRESPONSEOUTPUTPARSER_HPP
#define OPENAICHATCOMPLETIONRESPONSEOUTPUTPARSER_HPP

#include <instinct/ServerGlobals.hpp>
#include <instinct/output_parser/base_output_parser.hpp>
#include <instinct/llm_global.hpp>
#include <instinct/tools/chrono_utils.hpp>

namespace INSTINCT_SERVER_NS {
    using namespace INSTINCT_LLM_NS;

    /**
     * Utility class to transform a `Generation` to `OpenAIChatCompletionResponse`.
     */
    class ChatCompletionResponseOutputParser final: public BaseOutputParser<OpenAIChatCompletionResponse> {
    public:
        explicit ChatCompletionResponseOutputParser(const OutputParserOptions &options = {})
            : BaseOutputParser<OpenAIChatCompletionResponse>(options) {
        }

        OpenAIChatCompletionResponse ParseResult(const Generation& context) override {
            OpenAIChatCompletionResponse response;
            response.set_id(fmt::format("chatcmpl-{}", ChronoUtils::GetCurrentTimeMillis()));
            response.set_object("chat.completion");
            response.set_model("fake-model-by-instinct-server");
            response.set_created(ChronoUtils::GetCurrentTimeMillis() / 1000);
            response.set_system_fingerprint("fake-fingerprint-by-instinct-server");
            auto* choice = response.add_choices();
            choice->mutable_message()->set_content(context.message().content());
            choice->mutable_message()->set_role(context.message().role());
            choice->set_index(0);
            auto* usage = response.mutable_usage();
            usage->set_completion_tokens(0);
            usage->set_prompt_tokens(0);
            usage->set_total_tokens(0);
            return response;
        }

        std::string GetFormatInstruction() override {
            return "";
        }
    };


}

#endif //OPENAICHATCOMPLETIONRESPONSEOUTPUTPARSER_HPP
