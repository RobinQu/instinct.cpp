//
// Created by RobinQu on 2024/4/29.
//

#ifndef INPROCESSTASKSCHEDULER_HPP
#define INPROCESSTASKSCHEDULER_HPP

#include <blockingconcurrentqueue.h>

#include "ITaskScheduler.hpp"

namespace INSTINCT_DATA_NS {
    template<typename T>
    class InProcessTaskQueue final: public ITaskScheduler<T>::ITaskQueue {
        using Task = typename  ITaskScheduler<T>::Task;
        moodycamel::BlockingConcurrentQueue<Task> q_;
    public:
        InProcessTaskQueue() = default;

        void Enqueue(const Task &task) override {
            LOG_INFO("Enqueue task: id={},category={}", task.task_id, task.category);
            q_.enqueue(task);
        }

        bool Dequeue(Task &task) override {
            using namespace std::chrono_literals;
            // block until one task is enqueued with timeout
            return q_.wait_dequeue_timed(task, 3s);
        }

        std::vector<Task> Drain() override {
            std::vector<Task> tasks;
            Task buf[100];
            int count = 0;
            while((count = q_. try_dequeue_bulk(buf, 100)) != 0) {
                for(int i=0;i<count;++i) {
                    tasks.push_back(buf[i]);
                }
            }
            return tasks;
        }
    };

    template<typename T>
    static typename ITaskScheduler<T>::TaskQueuePtr CreateInProcessQueue() {
        return std::make_shared<InProcessTaskQueue<T>>();
    }

}


#endif //INPROCESSTASKSCHEDULER_HPP
