//
// Created by RobinQu on 2024/3/19.
//

#ifndef INSTINCT_ICHAINSERVER_HPP
#define INSTINCT_ICHAINSERVER_HPP
#include <string>

#include "ServerGlobals.hpp"
#include "chain/BaseChain.hpp"

namespace INSTINCT_SERVER_NS {
    using namespace INSTINCT_LLM_NS;

    template<typename ServerImpl>
    class IChainServer {
    public:
        virtual ~IChainServer() = default;
        IChainServer(IChainServer&&)=delete;
        IChainServer(const IChainServer&)=delete;
        IChainServer()=default;

        template<typename T>
        void AddChain(const std::string& endpoint, const ChainPtr<T>& chain) {

        }
    private:
        ServerImpl impl_;

    };


}

#endif //INSTINCT_ICHAINSERVER_HPP
