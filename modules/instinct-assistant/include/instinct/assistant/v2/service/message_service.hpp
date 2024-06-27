//
// Created by RobinQu on 2024/4/21.
//

#ifndef IMESSAGESERVICE_HPP
#define IMESSAGESERVICE_HPP

#include <instinct/assistant_api_v2.pb.h>

#include <instinct/assistant_global.hpp>

namespace INSTINCT_ASSISTANT_NS::v2 {

    class IMessageService {
    public:
        IMessageService()=default;
        virtual ~IMessageService()=default;
        IMessageService(IMessageService&&)=delete;
        IMessageService(const IMessageService&)=delete;

        virtual ListMessageResponse ListMessages(const ListMessagesRequest& list_request) = 0;
        virtual std::optional<MessageObject> CreateMessage(const CreateMessageRequest& create_request) = 0;
        virtual std::optional<MessageObject> CreateRawMessage(const MessageObject& message_object) = 0;
        virtual std::optional<MessageObject> RetrieveMessage(const GetMessageRequest& get_request) = 0;
        virtual std::optional<MessageObject> ModifyMessage(const ModifyMessageRequest& modify_request) = 0;

    };

    using MessageServicePtr = std::shared_ptr<IMessageService>;
}

#endif //IMESSAGESERVICE_HPP
