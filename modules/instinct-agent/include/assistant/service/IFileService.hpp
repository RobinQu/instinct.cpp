//
// Created by RobinQu on 2024/4/19.
//

#ifndef IFILESERVICE_HPP
#define IFILESERVICE_HPP

#include <assistant_api.pb.h>
#include "AgentGlobals.hpp"

namespace INSTINCT_AGENT_NS {

    class IFileService {
    public:
        IFileService()=default;
        virtual ~IFileService()=default;
        IFileService(IFileService&&)=delete;
        IFileService(const IFileService&)=delete;

    };

    using FileServicePtr = std::shared_ptr<IFileService>;

}

#endif //IFILESERVICE_HPP
