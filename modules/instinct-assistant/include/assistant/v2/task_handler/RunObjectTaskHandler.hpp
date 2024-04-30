//
// Created by RobinQu on 2024/4/29.
//

#ifndef RUNOBJECTHANDLER_HPP
#define RUNOBJECTHANDLER_HPP

#include "AssistantGlobals.hpp"
#include "task_scheduler/ThreadPoolTaskScheduler.hpp"

namespace INSTINCT_ASSISTANT_NS {
    using namespace INSTINCT_DATA_NS;

    class RunObjectTaskHandler final: public CommonTaskScheduler::ITaskHandler {
        DataMapperPtr<RunObject, std::string> run_data_mapper_;
        DataMapperPtr<RunStepObject, std::string> run_step_data_mapper_;
    public:
        static inline std::string CATEGORY = "run_object";

        bool Accept(const ITaskScheduler<std::string>::Task &task) override {
            return task.category == CATEGORY;
        }

        void Handle(const ITaskScheduler<std::string>::Task &task) override {

        }
    };
}


#endif //RUNOBJECTHANDLER_HPP
