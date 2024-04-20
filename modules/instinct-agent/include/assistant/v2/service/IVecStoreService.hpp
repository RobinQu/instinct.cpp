//
// Created by RobinQu on 2024/4/19.
//

#ifndef VECSTORESERVICE_HPP
#define VECSTORESERVICE_HPP

#include <assistant_api.pb.h>
#include "AgentGlobals.hpp"

namespace INSTINCT_AGENT_NS {

    class IVectorStoreService {
    public:
        IVectorStoreService()=default;
        virtual ~IVectorStoreService()=default;
        IVectorStoreService(IVectorStoreService&&)=delete;
        IVectorStoreService(const IVectorStoreService&)=delete;

    };

    using VectorStoreServicePtr = std::shared_ptr<IVectorStoreService>;

}

#endif //VECSTORESERVICE_HPP
