//
// Created by RobinQu on 2024/1/12.
//

#ifndef TYPES_H
#define TYPES_H
#include <any>
#include <unordered_map>
#include <exception>

#include <nlohmann/json.hpp>
#include <utility>
#include "CoreGlobals.hpp"


LC_CORE_NS {
    using OptionDict = nlohmann::json;
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

    class LangchainException: std::runtime_error {
        std::string reason_;

    public:
        explicit LangchainException(const std::string& basic_string)
            : std::runtime_error(basic_string), reason_(basic_string) {
        }

        explicit LangchainException(const char* const string)
            : std::runtime_error(string), reason_(string) {
        }

        explicit LangchainException(const runtime_error& runtime_error, std::string  basic_string)
            : std::runtime_error(runtime_error),reason_(std::move(basic_string)) {

        }

        [[nodiscard]] const char* what() const noexcept override {
            return reason_.data();
        }
    };


}




#endif //TYPES_H
