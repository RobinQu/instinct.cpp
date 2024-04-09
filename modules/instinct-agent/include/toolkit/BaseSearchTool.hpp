//
// Created by RobinQu on 2024/4/9.
//

#ifndef BASESEARCHTOOL_HPP
#define BASESEARCHTOOL_HPP


#include "AgentGlobals.hpp"
#include "ProtoMessageFunctionTool.hpp"

namespace INSTINCT_AGENT_NS {
    class BaseSearchTool: public ProtoMessageFunctionTool<SearchToolRequest, SearchToolResponse> {
    };
}

#endif //BASESEARCHTOOL_HPP
