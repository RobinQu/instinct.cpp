//
// Created by RobinQu on 2024/4/29.
//

#ifndef INPROCESSTASKSCHEDULER_HPP
#define INPROCESSTASKSCHEDULER_HPP

#include <moodycamel/concurrentqueue.h>

#include "ITaskScheduler.hpp"

namespace INSTINCT_DATA_NS {
    template<typename T>
    class InProcessTaskQueue final: public ITaskScheduler<T>::ITaskQueue {
        using Task = typename  ITaskScheduler<T>::Task;
        moodycamel::ConcurrentQueue<Task> q_;
    public:
        InProcessTaskQueue() = default;

        void Enqueue(const Task &task) override {
            LOG_INFO("Enqueue task: id={},category={}", task.task_id, task.category);
            q_.enqueue(task);
        }

        bool Dequeue(Task &task) override {
            return q_.try_dequeue(task);
        }
    };

    template<typename T>
    static typename ITaskScheduler<T>::TaskQueuePtr CreateInProcessQueue() {
        return std::make_shared<InProcessTaskQueue<T>>();
    }

}


#endif //INPROCESSTASKSCHEDULER_HPP
