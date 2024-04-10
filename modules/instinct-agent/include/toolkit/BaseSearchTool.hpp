//
// Created by RobinQu on 2024/4/9.
//

#ifndef BASESEARCHTOOL_HPP
#define BASESEARCHTOOL_HPP


#include "AgentGlobals.hpp"
#include "ProtoMessageFunctionTool.hpp"

namespace INSTINCT_AGENT_NS {
    class BaseSearchTool: public ProtoMessageFunctionTool<SearchToolRequest, SearchToolResponse> {
    public:
        explicit BaseSearchTool(const FunctionToolOptions &options)
            : ProtoMessageFunctionTool("Search", "A search engine. Useful for when you need to answer questions about current events. Input should be a search query.", options) {
        }
    };
}

#endif //BASESEARCHTOOL_HPP
