//
// Created by RobinQu on 2024/4/29.
//

#ifndef BASETASKSCHEDULER_HPP
#define BASETASKSCHEDULER_HPP

#include "DataGlobals.hpp"
#include "ITaskScheduler.hpp"


namespace INSTINCT_DATA_NS {

    template<typename T>
    class BaseTaskScheduler: public ITaskScheduler<T> {
        using TaskQueuePtr = typename ITaskScheduler<T>::TaskQueuePtr;
        using TaskHandlerPtr = typename ITaskScheduler<T>::TaskHandlerPtr;
        using TaskHandlerCallbacksPtr = typename BaseTaskScheduler::TaskHandlerCallbacksPtr;
        std::vector<TaskHandlerPtr> task_handlers_;
        TaskHandlerCallbacksPtr callbacks_;
    public:
        class NoOpTaskHandlerCallbacks: public ITaskScheduler<T>::ITaskHandlerCallbacks {
        public:
            void OnUnhandledTask(const typename ITaskScheduler<T>::Task &task) override {}

            void OnFailedTask(const typename ITaskScheduler<T>::TaskHandlerPtr &handler, const typename ITaskScheduler<T>::Task &task,
                const std::exception_ptr &error) override {}

            void OnHandledTask(const typename ITaskScheduler<T>::TaskHandlerPtr &handler,
                const typename ITaskScheduler<T>::Task &task) override {}
        };


        explicit BaseTaskScheduler(const TaskHandlerCallbacksPtr& callbacks): callbacks_(callbacks) {
            if (!callbacks_) {
                callbacks_ = std::make_shared<NoOpTaskHandlerCallbacks>();
            }
        }


        bool RegisterHandler(const TaskHandlerPtr &handler) override {
            task_handlers_.push_back(handler);
            return true;
        }

        bool RemoveHandler(const TaskHandlerPtr &handler) override {
            for (auto itr = task_handlers_.begin(); itr!=task_handlers_.end(); ++itr) {
                if (*itr == handler) {
                    task_handlers_.erase(itr);
                    return true;
                }
            }
            return false;
        }

        const std::vector<TaskHandlerPtr>& ListHandlers() const override {
            return task_handlers_;
        }

        TaskHandlerCallbacksPtr GetTaskHandlerCallbacks() const override {
            return callbacks_;
        }
    };
}


#endif //BASETASKSCHEDULER_HPP
