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


    TEST_F(TestThreadPoolTaskScheduler, TaskDispatchWithInMemoryQueue) {
        const auto task_scheduler =  CreateThreadPoolTaskScheduler(nullptr, 2);
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

        task_scheduler->Terminate().get();
    }
}
