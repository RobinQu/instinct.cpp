//
// Created by RobinQu on 2024/4/19.
//

#ifndef THREADSERVICE_HPP
#define THREADSERVICE_HPP

#include <assistant_api_v2.pb.h>
#include "AgentGlobals.hpp"

namespace INSTINCT_AGENT_NS::assistant::v2 {

    class IThreadService {
    public:
        IThreadService()=default;
        virtual ~IThreadService()=default;
        IThreadService(IThreadService&&)=delete;
        IThreadService(const IThreadService&)=delete;

        virtual ThreadObject CreateThread(const ThreadObject& create_request) = 0;
        virtual ThreadObject RetrieveThread(const RetrieveFileRequest& retrieve_request) = 0;
        virtual ThreadObject ModifyThread(const ModifyAssistantRequest& modify_request) = 0;
        virtual DeleteThreadResponse DeleteThread(const DeleteThreadResponse& delete_request) = 0;

    };

    using ThreadServicePtr = std::shared_ptr<IThreadService>;

}

#endif //THREADSERVICE_HPP
