//
// Created by RobinQu on 2024/5/3.
//

#ifndef INMEMORYSTATEMANAGER_HPP
#define INMEMORYSTATEMANAGER_HPP

#include "IStateManager.hpp"

namespace INSTINCT_LLM_NS {
    class InMemoryStateManager final: public IStateManager {
        std::unordered_map<std::string, AgentState> store_;
        std::unordered_map<std::string, std::mutex> mutexes_;
    public:
        std::future<AgentState> Load(const std::string &id) override {
            assert_not_blank(id, "id cannot be blank");
            return std::async(std::launch::async, [&] {
                assert_true(store_.contains(id), "Should contain agent state with id " + id);
                std::lock_guard lock{mutexes_[id]};
                return store_.at(id);
            });
        }

        std::future<void> Save(const AgentState &state) override {
            assert_not_blank(state.id(), "state should contain valid id");
            return std::async(std::launch::async, [&] {
                std::lock_guard lock{mutexes_[state.id()]};
                store_.emplace(state.id(), state);
            });
        }
    };
}


#endif //INMEMORYSTATEMANAGER_HPP
