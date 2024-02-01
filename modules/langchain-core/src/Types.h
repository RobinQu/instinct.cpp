//
// Created by RobinQu on 2024/1/12.
//

#ifndef TYPES_H
#define TYPES_H
#include <any>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace langchain::core {
    using OptionDict = nlohmann::ordered_json;
    struct Endpoint {
        std::string host;
        int port;
    };

    typedef std::unordered_map<std::string, std::string> HttpHeaders;

    enum HttpMethod {
        Unkown,
        GET,
        POST,
        PUT,
        DELETE,
        HEAD
    };


    class LangchainException final: std::runtime_error {
    public:
        explicit LangchainException(const std::string& basic_string)
            : runtime_error(basic_string) {
        }

        explicit LangchainException(const char* const string)
            : runtime_error(string) {
        }

        // explicit LangchainException(const runtime_error& runtime_error)
        //     : runtime_error(runtime_error) {
        // }
    };


}




#endif //TYPES_H
