//
// Created by RobinQu on 2024/4/19.
//

#ifndef IRUNSERVICE_HPP
#define IRUNSERVICE_HPP


#include <assistant_api.pb.h>
#include "AgentGlobals.hpp"

namespace INSTINCT_AGENT_NS {

    class IRunService {
    public:
        IRunService()=default;
        virtual ~IRunService()=default;
        IRunService(IRunService&&)=delete;
        IRunService(const IRunService&)=delete;

        // CreateAssistant();

    };

    using IRunServicePtr = std::shared_ptr<IRunService>;

}

#endif //IRUNSERVICE_HPP
