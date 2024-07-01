//
// Created by RobinQu on 2024/4/11.
//

#ifndef SERVERCONTROLLER_HPP
#define SERVERCONTROLLER_HPP

#include <instinct/server/mountable.hpp>
#include <instinct/server/server_lifecycle_handler.hpp>

namespace INSTINCT_SERVER_NS {
    class HttpLibServer;

    template<typename ServerImpl>
    class HttpController: public virtual IMountable<ServerImpl>, public virtual IServerLifeCycle<ServerImpl> {
    public:
        void Mount(ServerImpl &server) override {}

        void OnServerCreated(ServerImpl &server) override {}

        void OnServerStart(ServerImpl &server, int port) override {}

        void BeforeServerClose(ServerImpl &server) override {}

        void AfterServerClose(ServerImpl &server) override {}
    };


    template<typename ServerImpl>
    using HttpControllerPtr = std::shared_ptr<HttpController<ServerImpl>>;


}

#endif //SERVERCONTROLLER_HPP
