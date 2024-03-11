//
// Created by RobinQu on 2024/3/10.
//

#ifndef PROTOBUFMESSAGEOUTPUTPARSER_HPP
#define PROTOBUFMESSAGEOUTPUTPARSER_HPP

#include <google/protobuf/util/json_util.h>
#include "IOutputParser.hpp"
#include "LLMGlobals.hpp"
#include "tools/Assertions.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    template<typename T>
    requires is_pb_message<T>
    class ProtobufMessageOutputParser : public IOutputParser<T> {
    public:
        T ParseResult(const Generation& model_result) override { // NOLINT(*-convert-member-functions-to-static)
            T message;
            auto status = google::protobuf::json::JsonStringToMessage(model_result.has_message() ? model_result.message().content() : model_result.text(), &message);
            assert_true(status.ok());
            return message;
        }
    };
}

#endif //PROTOBUFMESSAGEOUTPUTPARSER_HPP
