//
// Created by RobinQu on 2024/1/14.
//

#ifndef SIMPLEHTTPCLIENT_H
#define SIMPLEHTTPCLIENT_H

#include <boost/beast.hpp>

#include "HttpCall.h"
#include "HttpResponse.h"


using namespace boost::beast;


namespace langchain::core {

class SimpleHttpClient {
    net::io_context ioc;
public:
    void DoExecute(
        const HttpCall& call,
        const std::function<std::string()>& request_body_function,
        const std::function<void(const HttpResponse&)>& response_body_function
    );
};

} // core
// langchain

#endif //SIMPLEHTTPCLIENT_H
