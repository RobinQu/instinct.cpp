//
// Created by RobinQu on 2024/4/11.
//

#ifndef ENDPOINTMOUNT_HPP
#define ENDPOINTMOUNT_HPP


#include <instinct/ServerGlobals.hpp>

namespace INSTINCT_SERVER_NS {

    template<typename ServerImpl>
    class IMountable {
    public:
        IMountable()=default;
        virtual ~IMountable()=default;
        IMountable(IMountable&&)=delete;
        IMountable(const IMountable&)=delete;

        /**
         * register mount in the implementation of this function
         * @param server
         */
        virtual void Mount(ServerImpl& server) = 0;
    };

    template<typename ServerImpl>
    using MountablePtr = std::shared_ptr<IMountable<ServerImpl>>;
}

#endif //ENDPOINTMOUNT_HPP
