//
// Created by RobinQu on 3/22/24.
//

#ifndef INSTINCT_PROTOBUFMESSAGEOUTPUTPARSER_HPP
#define INSTINCT_PROTOBUFMESSAGEOUTPUTPARSER_HPP


#include <google/protobuf/util/json_util.h>
#include "IOutputParser.hpp"
#include "LLMGlobals.hpp"
#include "tools/Assertions.hpp"
#include "output_parser/BaseOutputParser.hpp"
#include "prompt/MessageUtils.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;
    using namespace google::protobuf;

    template<typename T>
    requires is_pb_message<T>
    class ProtobufMessageOutputParser final : public BaseOutputParser<T> {
    public:
        std::string GetFormatInstruction() override {
            // TODO output json format instruction
            return "";
        }

        T ParseResult(const Generation &result) override {
            auto text = result.has_message() ? result.message().content() : result.text();
            T message;
            auto status = util::JsonStringToMessage(text, &message);
            assert_true(status.ok(), "failed to parse protobuf message from model output");
            return message;
        }
    };
}

#endif //INSTINCT_PROTOBUFMESSAGEOUTPUTPARSER_HPP
