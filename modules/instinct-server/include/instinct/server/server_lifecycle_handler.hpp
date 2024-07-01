//
// Created by RobinQu on 2024/4/11.
//

#ifndef ISERVERLIFECYCLEHANDLER_HPP
#define ISERVERLIFECYCLEHANDLER_HPP

#include <memory>
#include <instinct/server_global.hpp>


namespace INSTINCT_SERVER_NS {
    class HttpLibServer;

   template<typename ServerImpl>
    class IServerLifeCycle {
    public:
        IServerLifeCycle()=default;
        virtual ~IServerLifeCycle()=default;
        IServerLifeCycle(IServerLifeCycle&&)=delete;
        IServerLifeCycle(const IServerLifeCycle&)=delete;

        /**
         * Triggered when server is constructed
         * @param server
         */
        virtual void OnServerCreated(ServerImpl& server) = 0;

        /**
         * Triggered before server is listening
         * @param server
         * @param port
         */
        virtual void OnServerStart(ServerImpl& server, int port) = 0;

        /**
         * Trigger before server's close function is called
         * @param server
         */
        virtual void BeforeServerClose(ServerImpl& server) = 0;

        /**
         * Triggered after server's close function is called
         * @param server
         */
        virtual void AfterServerClose(ServerImpl& server) = 0;
    };

    using HttpLibServerLifeCyclePtr = std::shared_ptr<IServerLifeCycle<HttpLibServer>>;
}


#endif //ISERVERLIFECYCLEHANDLER_HPP
