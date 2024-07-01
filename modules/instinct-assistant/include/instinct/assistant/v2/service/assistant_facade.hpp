//
// Created by RobinQu on 2024/4/19.
//

#ifndef IASSISTANTAPIFACADE_HPP
#define IASSISTANTAPIFACADE_HPP

#include <instinct/assistant/v2/service/assistant_service.hpp>
#include <instinct/assistant/v2/service/file_service.hpp>
#include <instinct/assistant/v2/service/message_service.hpp>
#include <instinct/assistant/v2/service/run_service.hpp>
#include <instinct/assistant/v2/service/thread_service.hpp>
#include <instinct/assistant/v2/service/vector_store_service.hpp>


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
