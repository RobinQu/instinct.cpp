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

        virtual Impl& GetImpl() = 0;
        virtual Impl* operator->() const = 0;
        virtual std::chrono::time_point<std::chrono::system_clock> GetLastActiveTime() = 0;
        virtual void UpdateActiveTime() = 0;
    };


    template<typename Impl>
    class IConnectionPool {
    public:
        using ConnectionPtr = std::shared_ptr<IConnection<Impl>>;
        IConnectionPool()=default;
        virtual ~IConnectionPool()=default;
        IConnectionPool(const IConnectionPool&)=delete;
        IConnectionPool(IConnectionPool&&)=delete;

        virtual void Initialize() = 0;

        virtual ConnectionPtr Create() = 0;

        /**
         * return nullptr if no connection is available
         * @return
         */
        virtual ConnectionPtr TryAcquire() = 0;

        /**
         * throw if no connection is acquired
         * @return
         */
        virtual ConnectionPtr Acquire() = 0;

        /**
         * check a single connection
         * @param connection
         * @return
         */
        virtual bool Check(const ConnectionPtr& connection) = 0;

        /**
         * Release a single connection
         * @param connection
         */
        virtual void Release(const ConnectionPtr& connection) = 0;

    };
}

#endif //CONNECTIONPOOL_HPP
