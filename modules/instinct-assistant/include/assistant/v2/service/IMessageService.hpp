//
// Created by RobinQu on 2024/4/21.
//

#ifndef IMESSAGESERVICE_HPP
#define IMESSAGESERVICE_HPP

#include <assistant_api_v2.pb.h>

#include "AssistantGlobals.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {

    class IMessageService {
    public:
        IMessageService()=default;
        virtual ~IMessageService()=default;
        IMessageService(IMessageService&&)=delete;
        IMessageService(const IMessageService&)=delete;

        virtual ListMessageResponse ListMessages(const ListMessageRequest& list_request) = 0;
        virtual Message CreateMessage(const CreateMessageRequest& create_request) = 0;
        virtual Message RetrieveMessage(const GetMessageRequest& get_request) = 0;
        virtual Message ModifyMessagE(const ModifyMessageRequest& modify_request) = 0;

    };
}

#endif //IMESSAGESERVICE_HPP
