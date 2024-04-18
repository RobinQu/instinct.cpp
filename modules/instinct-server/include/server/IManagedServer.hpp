//
// Created by RobinQu on 2024/3/19.
//

#ifndef INSTINCT_ICHAINSERVER_HPP
#define INSTINCT_ICHAINSERVER_HPP
#include <string>

#include "CoreGlobals.hpp"
#include "HttpController.hpp"
#include "ServerGlobals.hpp"

namespace INSTINCT_SERVER_NS {
    using namespace INSTINCT_CORE_NS;

    template<typename ServerImpl>
    class IManagedServer {
    public:
        virtual ~IManagedServer() = default;
        IManagedServer(IManagedServer&&)=delete;
        IManagedServer(const IManagedServer&)=delete;
        IManagedServer()=default;

        virtual int Start() = 0;
        virtual bool StartAndWait() = 0;
        virtual void Shutdown() = 0;
        virtual void Use(const HttpControllerPtr<ServerImpl>& controller) = 0;
    };


}

#endif //INSTINCT_ICHAINSERVER_HPP
