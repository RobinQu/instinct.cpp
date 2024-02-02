//
// Created by RobinQu on 2024/1/14.
//

#ifndef SIMPLEHTTPCLIENT_H
#define SIMPLEHTTPCLIENT_H

#include <boost/beast.hpp>

#include "HttpRequest.h"
#include "HttpResponse.h"


using namespace boost::beast;


namespace langchain::core {

class SimpleHttpClient {
    net::io_context ioc;
public:
    HttpResponse DoExecute(
        const HttpRequest& call
    );

};

} // core
// langchain

#endif //SIMPLEHTTPCLIENT_H
