//
// Created by RobinQu on 3/19/24.
//

#ifndef INSTINCT_PLAINCHAINSERVER_HPP
#define INSTINCT_PLAINCHAINSERVER_HPP
#include <string>
#include <httplib.h>


#include "chain/BaseChain.hpp"
#include "ServerGlobals.hpp"


namespace INSTINCT_SERVER_NS {
    using namespace INSTINCT_LLM_NS;
    using namespace httplib;


    // IRunnable<Input,Output>
    // ChainContext {
    //  protobuf::any input
    // }
    //
    class PlainChainServer {
    public:
        template<class T>
        void AddRoute(const std::string& endpoint, const ChainPtr<T>& chain) {
//            server_.Post(endpoint, []);
        }

    private:
        Server server_;

    };

}


#endif //INSTINCT_PLAINCHAINSERVER_HPP
