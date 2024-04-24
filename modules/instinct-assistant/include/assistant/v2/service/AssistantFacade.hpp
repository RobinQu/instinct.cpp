//
// Created by RobinQu on 2024/4/19.
//

#ifndef IASSISTANTAPIFACADE_HPP
#define IASSISTANTAPIFACADE_HPP

#include "IAssistantService.hpp"
#include "IFileService.hpp"
#include "IRunService.hpp"
#include "IThreadService.hpp"
#include "IVectorStoreService.hpp"


namespace INSTINCT_ASSISTANT_NS::v2 {
    struct AssistantFacade {
        AssistantServicePtr assistant;
        FileServicePtr file;
        IRunServicePtr run;
        ThreadServicePtr thread;
        VectorStoreServicePtr vector_store;
    };
}

#endif //IASSISTANTAPIFACADE_HPP
