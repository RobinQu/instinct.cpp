//
// Created by RobinQu on 2024/2/1.
//

#include "HttpRestClient.h"

namespace langchain {
namespace core {
    template<typename Result>
    Result HttpRestClient::GetObject(const HttpRequest& call) {
        DoExecute(call, []() {return ""; }, [](const auto& resp) {

        });
    }

    template<typename Param, typename Result>
    Result HttpRestClient::GetObject(const HttpRequest& call, const Param& param) {
    }
} // core
} // langchain