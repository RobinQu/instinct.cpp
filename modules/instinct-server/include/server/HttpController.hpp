//
// Created by RobinQu on 2024/4/11.
//

#ifndef SERVERCONTROLLER_HPP
#define SERVERCONTROLLER_HPP

#include "IMountable.hpp"
#include "IServerLifecycleHandler.hpp"

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

    using HttpLibController = HttpController<HttpLibServer>;

    template<typename ServerImpl>
    using HttpControllerPtr = std::shared_ptr<HttpController<ServerImpl>>;

    using HttpLibControllerPtr = std::shared_ptr<HttpLibController>;

}

#endif //SERVERCONTROLLER_HPP
