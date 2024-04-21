//
// Created by RobinQu on 2024/4/19.
//

#ifndef IASSISTANTS_HPP
#define IASSISTANTS_HPP
#include <assistant_api_v2.pb.h>
#include "AgentGlobals.hpp"

namespace INSTINCT_AGENT_NS::assistant::v2 {

    class IAssistantService {
    public:
        IAssistantService()=default;
        virtual ~IAssistantService()=default;
        IAssistantService(IAssistantService&&)=delete;
        IAssistantService(const IAssistantService&)=delete;

        virtual ListAssistantsResponse ListAssistants(const ListAssistantsRequest& list_request) = 0;
        virtual AssistantObject CreateAssistant(const AssistantObject& create_request) = 0;
        virtual AssistantObject RetrieveAssistant(const GetAssistantRequest& get_request) = 0;
        virtual DeleteAssistantResponse DeleteAssistant(const DeleteAssistantRequest& delete_request) = 0;
        virtual AssistantObject ModifyAssistant(const ModifyAssistantRequest& modify_assistant_request) = 0;
    };

    using AssistantServicePtr = std::shared_ptr<IAssistantService>;

}

#endif //IASSISTANTS_HPP
