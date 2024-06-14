//
// Created by RobinQu on 2024/6/14.
//

#ifndef MANAGEDAPPLICATIONCONTEXT_HPP
#define MANAGEDAPPLICATIONCONTEXT_HPP

#include "CoreGlobals.hpp"

namespace INSTINCT_CORE_NS {

    static constexpr u_int32_t LOWEST_PRIORITY = 0;
    static constexpr u_int32_t LOW_PRIORITY = 1;
    static constexpr u_int32_t STANDARD_PRIORITY = 10;
    static constexpr u_int32_t MEDIUM_PRIORITY = 100;
    static constexpr u_int32_t HIGH_PRIORITY = 1000;
    static constexpr u_int32_t HIGHEST_PRIORITY = UINT32_MAX;


    class ILifeCycle {
    public:
        ILifeCycle()=default;
        virtual ~ILifeCycle()=default;
        ILifeCycle(ILifeCycle&&)=delete;
        ILifeCycle(const ILifeCycle&)=delete;
        virtual void Start() = 0;
        virtual void Stop() = 0;
        virtual u_int32_t GetPriority() = 0;
        virtual bool IsRunning()=0;
    };

    using LifeCycleObjectPtr = std::shared_ptr<ILifeCycle>;


    struct ManagedApplicationContext {
        std::vector<LifeCycleObjectPtr> life_cycle_managed;

        void Manage(const std::shared_ptr<void>& ptr) {
            if (const LifeCycleObjectPtr life_cycle = std::static_pointer_cast<ILifeCycle>(ptr)) {
                life_cycle_managed.push_back(life_cycle);
            }
        }

        void Startup() const {
            auto cmp = [](const LifeCycleObjectPtr& a, const LifeCycleObjectPtr& b) {
                return b->GetPriority() > a->GetPriority();
            };
            // build a PQ to select item with top priority
            std::priority_queue<LifeCycleObjectPtr, std::vector<LifeCycleObjectPtr>, decltype(cmp)> pq(cmp);
            for(const auto& object: life_cycle_managed) {
                pq.push(object);
            }
            while (!pq.empty()) {
                const auto item = pq.top();
                item->Start(); // will fail on exception
                pq.pop();
            }
        }

        void Shutdown() const {
            auto cmp = [](const LifeCycleObjectPtr& a, const LifeCycleObjectPtr& b) {
                return a->GetPriority() > b->GetPriority();
            };
            // build a PQ to select item with the least priority
            std::priority_queue<LifeCycleObjectPtr, std::vector<LifeCycleObjectPtr>, decltype(cmp)> pq(cmp);
            for(const auto& object: life_cycle_managed) {
                pq.push(object);
            }
            while (!pq.empty()) {
                const auto item = pq.top();
                try {
                    item->Stop();
                } catch (...) {
                    LOG_ERROR("Failed to stop a lifycycle managed object. Continue anyway.");
                }
                pq.pop();
            }
        }
    };
}

#endif //MANAGEDAPPLICATIONCONTEXT_HPP
