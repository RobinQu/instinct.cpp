//
// Created by RobinQu on 2024/4/19.
//

#ifndef IASSISTANTS_HPP
#define IASSISTANTS_HPP
#include <assistant_api.pb.h>
#include "AgentGlobals.hpp"

namespace INSTINCT_AGENT_NS {

    class IAssistantService {
    public:
        IAssistantService()=default;
        virtual ~IAssistantService()=default;
        IAssistantService(IAssistantService&&)=delete;
        IAssistantService(const IAssistantService&)=delete;

        // CreateAssistant();

    };

    using AssistantServicePtr = std::shared_ptr<IAssistantService>;

}

#endif //IASSISTANTS_HPP
