//
// Created by RobinQu on 2024/4/9.
//

#ifndef BASESEARCHTOOL_HPP
#define BASESEARCHTOOL_HPP


#include <instinct/LLMGlobals.hpp>
#include <instinct/toolkit/ProtoMessageFunctionTool.hpp>

namespace INSTINCT_LLM_NS {
    class BaseSearchTool: public ProtoMessageFunctionTool<SearchToolRequest, SearchToolResponse> {
    public:
        explicit BaseSearchTool(const FunctionToolOptions &options, const std::string& name = "Search", const std::string& description = "A search engine. Useful for when you need to answer questions about current events. Input should be a search query.")
            : ProtoMessageFunctionTool(name, description, options) {
        }

        std::string GetExample() override {
            return GetSchema().name() + R"(({"query":"keyword}))";
        }
    };
}

#endif //BASESEARCHTOOL_HPP
