//
// Created by RobinQu on 2024/4/29.
//
#include <gtest/gtest.h>

#include "DataGlobals.hpp"
#include "task_scheduler/ThreadPoolTaskScheduler.hpp"
#include "tools/RandomUtils.hpp"

namespace INSTINCT_DATA_NS {
    using namespace std::chrono_literals;

    class TestThreadPoolTaskScheduler: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
        }
    };

    class HandlerA final: public CommonTaskScheduler::ITaskHandler {
    public:
        std::atomic_int c;
        bool Accept(const ITaskScheduler<std::string>::Task &task) override {
            return task.category == "a";
        }

        void Handle(const ITaskScheduler<std::string>::Task &task) override {
            LOG_INFO("handle task: id={}, category={}", task.task_id, task.category);
            c.fetch_add(1);
        }
    };

    class HandlerB final: public CommonTaskScheduler::ITaskHandler {
    public:
        std::atomic_int c;
        bool Accept(const ITaskScheduler<std::string>::Task &task) override {
            return task.category == "b";
        }

        void Handle(const ITaskScheduler<std::string>::Task &task) override {
            LOG_INFO("handle task: id={}, category={}", task.task_id, task.category);
            c.fetch_add(1);
        }
    };

    class HandlerC final: public CommonTaskScheduler::ITaskHandler {
    public:
        bool Accept(const ITaskScheduler<std::string>::Task &task) override {
            return task.category == "c";
        }

        void Handle(const ITaskScheduler<std::string>::Task &task) override {
            LOG_INFO("handle task: id={}, category={}", task.task_id, task.category);
            std::this_thread::sleep_for(3s);
        }
    };

    class HandlerD final: public CommonTaskScheduler::ITaskHandler {
    public:
        std::atomic_int c;
        bool Accept(const ITaskScheduler<std::string>::Task &task) override {
            return task.category == "d";
        }

        void Handle(const ITaskScheduler<std::string>::Task &task) override {
            LOG_INFO("handle task: id={}, category={}", task.task_id, task.category);
            c.fetch_add(1);
            throw std::runtime_error("boom");
        }
    };

    class CountingCallbacks final: public CommonTaskScheduler::ITaskHandlerCallbacks {
    public:
        std::atomic_int unhandled = 0;
        std::atomic_int failed = 0;
        std::atomic_int handled = 0;

        void OnUnhandledTask(const ITaskScheduler<std::string>::Task &task) override {
            ++unhandled;
        }

        void OnFailedTask(const ITaskScheduler<std::string>::TaskHandlerPtr &handler,
            const ITaskScheduler<std::string>::Task &task, const std::exception_ptr &error) override {
            ++failed;
        }

        void OnHandledTask(const ITaskScheduler<std::string>::TaskHandlerPtr &handler,
            const ITaskScheduler<std::string>::Task &task) override {
            ++handled;
        }
    };


    TEST_F(TestThreadPoolTaskScheduler, TaskDispatchWithInMemoryQueue) {
        const auto task_scheduler =  CreateThreadPoolTaskScheduler(2);
        auto h1 = std::make_shared<HandlerA>();
        auto h2 = std::make_shared<HandlerB>();
        task_scheduler->RegisterHandler(h1);
        task_scheduler->RegisterHandler(h2);
        task_scheduler->Start();

        int n = 10;
        while (n-->0) {
            task_scheduler->Enqueue({
                .task_id = StringUtils::GenerateUUIDString(),
                .category =  RandomUtils::GetRandom() > 0.5 ? "a" : "b"
            });
        }

        std::this_thread::sleep_for(3s);
        ASSERT_EQ(h1->c + h2->c, 10);
        const auto drained = task_scheduler->Terminate().get();
        ASSERT_TRUE(drained.empty());
    }

    TEST_F(TestThreadPoolTaskScheduler, DrainTasks) {
        constexpr int thread_count = 2, task_count = 10;
        const auto task_scheduler = CreateThreadPoolTaskScheduler(thread_count);
        task_scheduler->RegisterHandler(std::make_shared<HandlerC>());
        task_scheduler->Start();
        int n = task_count;
        while (n-->0) {
            task_scheduler->Enqueue({.task_id = StringUtils::GenerateUUIDString(), .category = "c"});
        }

        if (const auto& thread_pool_scheduler = std::dynamic_pointer_cast<ThreadPoolTaskScheduler<std::string>>(task_scheduler)) {
            const auto drained = thread_pool_scheduler->GetQueue()->Drain();
            ASSERT_EQ(drained.size(), task_count-thread_count);

            const auto drained2 = task_scheduler->Terminate().get();
            ASSERT_TRUE(drained2.empty());
        }
    }

    TEST_F(TestThreadPoolTaskScheduler, Callbacks) {
        const auto callbacks = std::make_shared<CountingCallbacks>();
        const auto task_scheduler =  CreateThreadPoolTaskScheduler(2, nullptr, callbacks);
        const auto ha = std::make_shared<HandlerA>();
        const auto hb = std::make_shared<HandlerB>();
        const auto hd = std::make_shared<HandlerD>();

        task_scheduler->RegisterHandler(ha);
        task_scheduler->RegisterHandler(hb);
        task_scheduler->RegisterHandler(hd);
        task_scheduler->Start();

        const std::vector nums = {5,4,6,6};
        int num_a = nums[0];
        int num_b = nums[1];
        int num_c = nums[2];
        int num_d = nums[3];

        while (num_a-->0) {
            task_scheduler->Enqueue({.task_id = StringUtils::GenerateUUIDString(), .category = "a"});
        }
        while (num_b-->0) {
            task_scheduler->Enqueue({.task_id = StringUtils::GenerateUUIDString(), .category = "b"});
        }
        while (num_c-->0) {
            task_scheduler->Enqueue({.task_id = StringUtils::GenerateUUIDString(), .category = "c"});
        }
        while (num_d-->0) {
            task_scheduler->Enqueue({.task_id = StringUtils::GenerateUUIDString(), .category = "d"});
        }

        std::this_thread::sleep_for(3s);

        ASSERT_EQ(ha->c, nums[0]);
        ASSERT_EQ(hb->c, nums[1]);

        ASSERT_EQ(callbacks->handled, ha->c + hb->c);
        ASSERT_EQ(callbacks->unhandled, nums[2]);
        ASSERT_EQ(callbacks->failed, hd->c);
    }

}
