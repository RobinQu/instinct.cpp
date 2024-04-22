//
// Created by RobinQu on 2024/4/22.
//

#ifndef CONNECTIONPOOL_HPP
#define CONNECTIONPOOL_HPP

#include "RetrievalGlobals.hpp"

namespace INSTINCT_RETRIEVAL_NS {

    template<typename Impl>
    class IConnection {
    public:
        IConnection()=default;
        virtual ~IConnection()=default;
        IConnection(IConnection&&)=delete;
        IConnection(const IConnection&)=delete;

        virtual Impl* operator*() = 0;
        virtual bool IsAlive() = 0;
        virtual std::chrono::time_point<std::chrono::system_clock> GetLastActiveTime() = 0;
        virtual void UpdateActiveTime() = 0;
    };


    template<typename Impl>
    class IConnectionPool {
    public:
        IConnectionPool()=default;
        virtual ~IConnectionPool()=default;
        IConnectionPool(const IConnectionPool&)=delete;
        IConnectionPool(IConnectionPool&&)=delete;

        virtual std::shared_ptr<IConnection<Impl>> Create() = 0;
        virtual std::shared_ptr<IConnection<Impl>> Acquire() = 0;
        virtual void Release(const std::shared_ptr<IConnection<Impl>>& connection) = 0;
    };
}

#endif //CONNECTIONPOOL_HPP
