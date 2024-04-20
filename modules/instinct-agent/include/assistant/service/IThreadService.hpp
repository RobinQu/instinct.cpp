//
// Created by RobinQu on 2024/4/19.
//

#ifndef THREADSERVICE_HPP
#define THREADSERVICE_HPP

#include <assistant_api.pb.h>
#include "AgentGlobals.hpp"

namespace INSTINCT_AGENT_NS {

    class IThreadService {
    public:
        IThreadService()=default;
        virtual ~IThreadService()=default;
        IThreadService(IThreadService&&)=delete;
        IThreadService(const IThreadService&)=delete;

        // CreateAssistant();

    };

    using ThreadServicePtr = std::shared_ptr<IThreadService>;

}

#endif //THREADSERVICE_HPP
