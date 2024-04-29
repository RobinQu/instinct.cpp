//
// Created by RobinQu on 2024/4/29.
//

#ifndef THREADPOOLTASKSCHEDULER_HPP
#define THREADPOOLTASKSCHEDULER_HPP
#include <thread>

#include "BaseTaskScheduler.hpp"
#include "DataGlobals.hpp"
#include "InProcessTaskQueue.hpp"

namespace INSTINCT_DATA_NS {
    template<typename T>
    class ThreadPoolTaskScheduler final: public BaseTaskScheduler<T> {
        unsigned int consumer_thread_count_;
        std::vector<std::thread> consumer_threads_;
        volatile bool running_;

    public:
        using Task = typename ITaskScheduler<T>::Task;
        using TaskQueuePtr = typename ITaskScheduler<T>::TaskQueuePtr;

        ThreadPoolTaskScheduler(
            const TaskQueuePtr &queue,
            const unsigned int consumer_thread_count):
            BaseTaskScheduler<T>(queue),
            consumer_thread_count_(consumer_thread_count),
            running_(false) {
        }

        ~ThreadPoolTaskScheduler() override {
            Terminate().get();
        }

        void Start() override {
            int n = consumer_thread_count_;
            running_ = true;
            while (n-->0) {
                consumer_threads_.emplace_back([&] {
                    while (running_) {
                        if(Task task; this->GetQueue()->Dequeue(task)) {
                            for (const auto& handler: this->ListHandlers()) {
                                if(handler->Accept(task)) {
                                    handler->Handle(task);
                                }
                            }
                        }
                    }
                });
            }
        }

        std::future<void> Terminate() override {
            return std::async(std::launch::async, [&] {
                for(auto& t: consumer_threads_) {
                    if(t.joinable()) {
                        t.join();
                    }
                }
            });
        }
    };

    using CommonTaskScheduler = ITaskScheduler<std::string>;
    using CommonTaskSchedulerPtr = TaskSchedulerPtr<std::string>;

    template<typename Payload=std::string>
    static TaskSchedulerPtr<Payload> CreateThreadPoolTaskScheduler(
        const typename ThreadPoolTaskScheduler<Payload>::TaskQueuePtr& task_queue = nullptr,
        const unsigned int consumer_thread_count = std::thread::hardware_concurrency()) {
        if (!task_queue) {
            task_queue = CreateInProcessQueue<Payload>();
        }
        return std::make_shared<ThreadPoolTaskScheduler<Payload>>(task_queue, consumer_thread_count);
    }

}

#endif //THREADPOOLTASKSCHEDULER_HPP
