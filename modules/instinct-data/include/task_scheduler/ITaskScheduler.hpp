//
// Created by RobinQu on 2024/4/29.
//

#ifndef ITASKSCHEDULER_HPP
#define ITASKSCHEDULER_HPP

#include "DataGlobals.hpp"


namespace INSTINCT_DATA_NS {
    /**
     * A skeleton interface for multi-consumer task queue
     * @tparam Payload
     */
    template<typename Payload>
    class ITaskScheduler {
    public:
        struct Task {
            std::string task_id;
            std::string category;
            Payload payload;
        };

        class ITaskHandler {
        public:
            ITaskHandler(ITaskHandler&&)=delete;
            ITaskHandler(const ITaskHandler&)=delete;
            ITaskHandler()=default;
            virtual ~ITaskHandler()=default;

            virtual bool Accept(const Task& task) = 0;
            virtual void Handle(const Task& task) = 0;
        };
        using TaskHandlerPtr = std::shared_ptr<ITaskHandler>;

        class ITaskQueue {
        public:
            ITaskQueue(const ITaskQueue&)=delete;
            ITaskQueue(ITaskQueue&&)=delete;
            ITaskQueue()=default;
            virtual ~ITaskQueue()=default;

            virtual void Enqueue(const Task& task) = 0;
            virtual bool Dequeue(Task& task) = 0;
            virtual std::vector<Task> Drain() = 0;
        };
        using TaskQueuePtr = std::shared_ptr<ITaskQueue>;

        class ITaskHandlerCallbacks {
        public:
            ITaskHandlerCallbacks()=default;
            virtual ~ITaskHandlerCallbacks()=default;
            ITaskHandlerCallbacks(const ITaskHandlerCallbacks&)=delete;
            ITaskHandlerCallbacks(ITaskHandlerCallbacks&&)=delete;
            virtual void OnUnhandledTask(const Task& task) = 0;
            virtual void OnFailedTask(const TaskHandlerPtr& handler, const Task& task, std::runtime_error& error) = 0;
            virtual void OnHandledTask(const TaskHandlerPtr& handler, const Task& task) = 0;
        };
        using TaskHandlerCallbacksPtr = std::shared_ptr<ITaskHandlerCallbacks>;

        ITaskScheduler(const ITaskScheduler&)=delete;
        ITaskScheduler(ITaskScheduler&&)=delete;
        virtual ~ITaskScheduler()=default;
        ITaskScheduler() = default;

        virtual bool RegisterHandler(const TaskHandlerPtr& handler)=0;
        virtual bool RemoveHandler(const TaskHandlerPtr& handler)=0;
        virtual const std::vector<TaskHandlerPtr>& ListHandlers() const=0;
        virtual TaskHandlerCallbacksPtr GetTaskHandlerCallbacks() const=0;

        /**
         * A shortcut method to add a task to queue
         * @param task
         */
        virtual void Enqueue(const Task& task) = 0;

        /**
         * Start this scheduler
         */
        virtual void Start() = 0;

        /**
         * Stop scheduler
         * @return
         */
        virtual std::future<std::vector<Task>> Terminate() = 0;
    };

    template<typename Payload>
    using TaskSchedulerPtr = std::shared_ptr<ITaskScheduler<Payload>>;
}

#endif //ITASKSCHEDULER_HPP
