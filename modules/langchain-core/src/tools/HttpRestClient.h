//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPRESTCLIENT_H
#define HTTPRESTCLIENT_H
#include "SimpleHttpClient.h"


namespace langchain::core {

class HttpRestClient final: SimpleHttpClient {

    template<typename Result>
    Result GetObject(const HttpRequest& call);

    template<typename Param, typename Result>
    Result GetObject(const HttpRequest& call, const Param& param);

};

} // core
// langchain

#endif //HTTPRESTCLIENT_H
