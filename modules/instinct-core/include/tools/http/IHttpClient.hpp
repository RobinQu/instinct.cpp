//
// Created by RobinQu on 2024/3/18.
//

#ifndef INSTINCT_IHTTPCLIENT_HPP
#define INSTINCT_IHTTPCLIENT_HPP

#include "CoreGlobals.hpp"
#include "tools/HttpResponse.hpp"
#include "tools/HttpRequest.hpp"
#include "functional/ReactiveFunctions.hpp"

namespace INSTINCT_CORE_NS  {
    class IHttpClient final {
    public:
        virtual ~IHttpClient() = default;
        IHttpClient(const IHttpClient&)=delete;
        IHttpClient(IHttpClient&&)=delete;

        virtual HttpResponse Execute(
                const HttpRequest& call
        ) = 0;

        virtual AsyncIterator<std::string> StreamChunk(
                const HttpRequest& call
        ) = 0;


    };
}


#endif //INSTINCT_IHTTPCLIENT_HPP
