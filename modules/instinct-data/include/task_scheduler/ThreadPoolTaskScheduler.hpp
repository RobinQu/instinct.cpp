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
        using TaskHandlerCallbacksPtr = typename ITaskScheduler<T>::TaskHandlerCallbacksPtr;

        ThreadPoolTaskScheduler(
            const TaskQueuePtr &queue,
            const TaskHandlerCallbacksPtr& callbacks,
            const unsigned int consumer_thread_count):
            BaseTaskScheduler<T>(queue, callbacks),
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
                            bool handled = false;
                            for (const auto& handler: this->ListHandlers()) {
                                try {
                                    if(handler->Accept(task)) {
                                        handled = true;
                                        handler->Handle(task);

                                        try {
                                            this->GetTaskHandlerCallbacks()->OnHandledTask(handler, task);
                                        } catch (...) {}
                                    }
                                } catch (std::runtime_error& e) {
                                    LOG_WARN("failed task found: id={}, category={}, e.what={}", task.task_id, task.category, e.what());
                                    try {
                                        this->GetTaskHandlerCallbacks()->OnFailedTask(handler, task, e);
                                    } catch (...) {}
                                }
                            }
                            LOG_WARN("unhandled task found: id={}, category={}", task.task_id, task.category);
                            if (!handled) {
                                try {
                                    this->GetTaskHandlerCallbacks()->OnUnhandledTask(task);
                                } catch (...) {}
                            }
                        }
                    }
                });
            }
        }

        std::future<std::vector<Task>> Terminate() override {
            return std::async(std::launch::async, [&] {
                running_ = false;
                for(auto& t: consumer_threads_) {
                    if(t.joinable()) {
                        t.join();
                    }
                }
                return this->GetQueue()->Drain();
            });
        }
    };

    using CommonTaskScheduler = ITaskScheduler<std::string>;
    using CommonTaskSchedulerPtr = TaskSchedulerPtr<std::string>;

    template<typename Payload=std::string>
    static TaskSchedulerPtr<Payload> CreateThreadPoolTaskScheduler(
        const unsigned int consumer_thread_count = std::thread::hardware_concurrency(),
        typename ThreadPoolTaskScheduler<Payload>::TaskQueuePtr task_queue = nullptr,
        const typename ThreadPoolTaskScheduler<Payload>::TaskHandlerCallbacksPtr& task_handler_callbacks = nullptr) {
        if (!task_queue) {
            task_queue = CreateInProcessQueue<Payload>();
        }
        return std::make_shared<ThreadPoolTaskScheduler<Payload>>(task_queue, task_handler_callbacks, consumer_thread_count);
    }

}

#endif //THREADPOOLTASKSCHEDULER_HPP
