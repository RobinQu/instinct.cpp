//
// Created by RobinQu on 2024/3/19.
//

#ifndef INSTINCT_ICHAINSERVER_HPP
#define INSTINCT_ICHAINSERVER_HPP
#include <string>

#include "CoreGlobals.hpp"
#include "ServerGlobals.hpp"

namespace INSTINCT_SERVER_NS {
    using namespace INSTINCT_CORE_NS;

    class IManagedServer;
    // using ServerInitCallback = std::function<void(IManagedServer&)>;
    // using ServerStartedCallback = std::function<void(IManagedServer&)>;
    // using ServerShutdownCallback = std::function<void(IManagedServer&)>;
    // using CallbackRefID = int32_t;

    class IManagedServer {
    public:
        virtual ~IManagedServer() = default;
        IManagedServer(IManagedServer&&)=delete;
        IManagedServer(const IManagedServer&)=delete;
        IManagedServer()=default;

        virtual int Start() = 0;
        virtual bool StartAndWait() = 0;
        virtual void Shutdown() = 0;

        // virtual CallbackRefID OnServerInit(const ServerInitCallback& callback);
        // virtual CallbackRefID OnServerStarted(const ServerStartedCallback& callback);
        // virtual CallbackRefID ServerShutdownCallback(const ServerShutdownCallback& callback);
        //
    };


}

#endif //INSTINCT_ICHAINSERVER_HPP
