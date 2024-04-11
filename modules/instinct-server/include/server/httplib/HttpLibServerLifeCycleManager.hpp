//
// Created by RobinQu on 2024/4/11.
//

#ifndef DOINGNOTHINGSERVERLIFECYCLE_HPP
#define DOINGNOTHINGSERVERLIFECYCLE_HPP

#include <vector>
#include "../IServerLifecycleHandler.hpp"


namespace INSTINCT_SERVER_NS {
    class HttpLibServerLifeCycleManager final: public IServerLifeCycle<HttpLibServer> {
        std::vector<HttpLibServerLifeCyclePtr> server_life_cycles_;
    public:
        void AddServerLifeCylce(const HttpLibServerLifeCyclePtr& server_life_cycle) {
            server_life_cycles_.push_back(server_life_cycle);
        }

        void OnServerCreated(HttpLibServer &server) override {
            for(const auto& server_life_cycle: server_life_cycles_) {
                server_life_cycle->OnServerCreated(server);
            }
        }

        void OnServerStart(HttpLibServer &server, const int port) override {
            for(const auto& server_life_cycle: server_life_cycles_) {
                server_life_cycle->OnServerStart(server, port);
            }
        }

        void BeforeServerClose(HttpLibServer &server) override {
            for(const auto& server_life_cycle: server_life_cycles_) {
                server_life_cycle->BeforeServerClose(server);
            }
        }

        void AfterServerClose(HttpLibServer &server) override {
            for(const auto& server_life_cycle: server_life_cycles_) {
                server_life_cycle->AfterServerClose(server);
            }
        }

    };
}

#endif //DOINGNOTHINGSERVERLIFECYCLE_HPP
