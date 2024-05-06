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

    public:
        using Task = typename ITaskScheduler<T>::Task;
        using TaskQueuePtr = typename ITaskScheduler<T>::TaskQueuePtr;
        using TaskHandlerCallbacksPtr = typename ITaskScheduler<T>::TaskHandlerCallbacksPtr;

    private:
        unsigned int consumer_thread_count_;
        std::vector<std::thread> consumer_threads_;
        volatile bool running_;
        TaskQueuePtr queue_;
    public:


        ThreadPoolTaskScheduler(
            const TaskQueuePtr &queue,
            const TaskHandlerCallbacksPtr& callbacks,
            const unsigned int consumer_thread_count):
            BaseTaskScheduler<T>(callbacks),
            consumer_thread_count_(consumer_thread_count),
            running_(false),
            queue_(queue) {
        }

        ~ThreadPoolTaskScheduler() override {
            Terminate().get();
        }

        void Start() override {
            int n = consumer_thread_count_;
            LOG_INFO("ThreadPoolTaskScheduler started with {} threads", n);
            running_ = true;
            while (n-->0) {
                consumer_threads_.emplace_back([&] {
                    while (running_) {
                        if(Task task; queue_->Dequeue(task)) {
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

        TaskQueuePtr GetQueue() const {
            return queue_;
        }

        void Enqueue(const Task &task) override {
            queue_->Enqueue(task);
        }

        std::future<std::vector<Task>> Terminate() override {
            return std::async(std::launch::async, [&] {
                running_ = false;
                for(auto& t: consumer_threads_) {
                    if(t.joinable()) {
                        t.join();
                    }
                }
                return queue_->Drain();
            });
        }
    };

    using CommonTaskScheduler = ITaskScheduler<std::string>;
    using CommonTaskSchedulerPtr = TaskSchedulerPtr<std::string>;

    template<typename Payload>
    std::set<TaskSchedulerPtr<Payload>> TASK_SCHEDULERS;

    template<typename Payload=std::string>
    static TaskSchedulerPtr<Payload> CreateThreadPoolTaskScheduler(
        const unsigned int consumer_thread_count = std::thread::hardware_concurrency(),
        typename ThreadPoolTaskScheduler<Payload>::TaskQueuePtr task_queue = nullptr,
        const typename ThreadPoolTaskScheduler<Payload>::TaskHandlerCallbacksPtr& task_handler_callbacks = nullptr) {
        if (!task_queue) {
            task_queue = CreateInProcessQueue<Payload>();
        }
        const auto scheduler = std::make_shared<ThreadPoolTaskScheduler<Payload>>(task_queue, task_handler_callbacks, consumer_thread_count);
        TASK_SCHEDULERS<Payload>.insert(scheduler);
        return scheduler;
    }

    template<typename Payload=std::string>
    static void GracefullyShutdownThreadPoolTaskSchedulers() {
        for(const auto& schedueler: TASK_SCHEDULERS<Payload>) {
            if (const auto ret = schedueler->Terminate().get();!ret.empty()) {
                LOG_INFO("Scheduler terminated with {} remaining task(s)", ret.size());
            }
        }
    }

}

#endif //THREADPOOLTASKSCHEDULER_HPP
