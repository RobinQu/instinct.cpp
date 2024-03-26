//
// Created by RobinQu on 2024/3/26.
//

#ifndef OPENAICHATCOMPLETIONREQUESTINPUTPARSER_HPP
#define OPENAICHATCOMPLETIONREQUESTINPUTPARSER_HPP

#include "ServerGlobals.hpp"
#include "input_parser/BaseInputParser.hpp"
#include "LLMGlobals.hpp"

namespace INSTINCT_SERVER_NS {
    using namespace INSTINCT_LLM_NS;

    struct OpenAIChatCompletionInputParserOptions {
        InputParserOptions base_options;
        std::string chat_history_variable_key = "chat_history";
        std::string question_variable_key = "question";
    };

    /**
     * Utility class to transform `OpenAIChatCompletionRequest` to a `JSONMappingContext`.
     * In converted `MappingData`, there should contain `chat_history` of `MesasgeList` type and `question` of `std::string` type.
     *
     */
    class OpenAIChatCompletionRequestInputParser final: public BaseInputParser<OpenAIChatCompletionRequest> {
        OpenAIChatCompletionInputParserOptions options_;

    public:
        explicit OpenAIChatCompletionRequestInputParser(const OpenAIChatCompletionInputParserOptions& options = {})
            : BaseInputParser<OpenAIChatCompletionRequest>(options.base_options),
              options_(options) {
        }

        JSONContextPtr ParseInput(const OpenAIChatCompletionRequest& input) override {
            JSONMappingContext mapping_data;
            // treat last message as latest user input
            const auto question_ctx = CreateJSONContext();
            question_ctx->ProducePrimitive(input.messages().end()->content());
            mapping_data[options_.question_variable_key] = question_ctx;

            // treat first n-1 messages as chat_history
            MessageList message_list;
            for(int i=0;i<input.messages_size()-1;++i) {
                message_list.add_messages()->CopyFrom(input.messages(i));
            }
            const auto msg_list_ctx = CreateJSONContext();
            msg_list_ctx->ProduceMessage(message_list);
            mapping_data[options_.chat_history_variable_key] = msg_list_ctx;

            // return mapping data
            auto context = CreateJSONContext();
            context->ProduceMappingData(mapping_data);
            return context;
        }
    };
}

#endif //OPENAICHATCOMPLETIONREQUESTINPUTPARSER_HPP
