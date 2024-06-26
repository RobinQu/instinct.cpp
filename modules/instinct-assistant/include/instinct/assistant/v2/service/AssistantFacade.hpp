//
// Created by RobinQu on 2024/4/19.
//

#ifndef IASSISTANTAPIFACADE_HPP
#define IASSISTANTAPIFACADE_HPP

#include <instinct/assistant/v2/service/IAssistantService.hpp>
#include <instinct/assistant/v2/service/IFileService.hpp>
#include <instinct/assistant/v2/service/IMessageService.hpp>
#include <instinct/assistant/v2/service/IRunService.hpp>
#include <instinct/assistant/v2/service/IThreadService.hpp>
#include <instinct/assistant/v2/service/IVectorStoreService.hpp>


namespace INSTINCT_ASSISTANT_NS::v2 {
    struct AssistantFacade {
        AssistantServicePtr assistant;
        FileServicePtr file;
        RunServicePtr run;
        ThreadServicePtr thread;
        MessageServicePtr message;
        VectorStoreServicePtr vector_store;
    };
}

#endif //IASSISTANTAPIFACADE_HPP
