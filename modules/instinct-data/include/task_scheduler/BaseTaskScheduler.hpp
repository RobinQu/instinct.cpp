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
        TaskQueuePtr queue_;
        std::vector<TaskHandlerPtr> task_handlers_;
    public:
        explicit BaseTaskScheduler(const TaskQueuePtr &queue)
            : queue_(queue) {
        }

        TaskQueuePtr GetQueue() const override {
            return queue_;
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

    };
}


#endif //BASETASKSCHEDULER_HPP
