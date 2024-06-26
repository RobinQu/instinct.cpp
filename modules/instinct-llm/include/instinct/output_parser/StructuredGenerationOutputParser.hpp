//
// Created by RobinQu on 2024/3/10.
//

#ifndef PROTOBUFMESSAGEOUTPUTPARSER_HPP
#define PROTOBUFMESSAGEOUTPUTPARSER_HPP

#include <google/protobuf/util/json_util.h>
#include "LLMGlobals.hpp"
#include "tools/Assertions.hpp"
#include "BaseOutputParser.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;
    using namespace google::protobuf;

    template<typename T>
    requires IsProtobufMessage<T>
    class StructuredGenerationOutputParser final : public BaseOutputParser<T> {
    public:
        T ParseResult(const Generation& model_result) override { // NOLINT(*-convert-member-functions-to-static)
            T message;
            auto status = util::JsonStringToMessage(model_result.has_message() ? model_result.message().content() : model_result.text(), &message);
            assert_true(status.ok());
            return message;
        }

        std::string GetFormatInstruction() override {
            // TODO
            return "";
        }
    };
}

#endif //PROTOBUFMESSAGEOUTPUTPARSER_HPP
