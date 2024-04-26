//
// Created by RobinQu on 2024/4/19.
//

#ifndef THREADSERVICE_HPP
#define THREADSERVICE_HPP

#include <assistant_api_v2.pb.h>
#include "AssistantGlobals.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {

    class IThreadService {
    public:
        IThreadService()=default;
        virtual ~IThreadService()=default;
        IThreadService(IThreadService&&)=delete;
        IThreadService(const IThreadService&)=delete;

        virtual std::optional<ThreadObject> CreateThread(const ThreadObject& create_request) = 0;
        virtual std::optional<ThreadObject> RetrieveThread(const GetThreadRequest& retrieve_request) = 0;
        virtual std::optional<ThreadObject> ModifyThread(const ModifyThreadRequest& modify_request) = 0;
        virtual DeleteThreadResponse DeleteThread(const DeleteThreadRequest& delete_request) = 0;

    };

    using ThreadServicePtr = std::shared_ptr<IThreadService>;

}

#endif //THREADSERVICE_HPP
