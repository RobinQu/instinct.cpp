//
// Created by RobinQu on 2024/2/13.
//

#ifndef HTTPCLIENTEXCEPTION_H
#define HTTPCLIENTEXCEPTION_H

#include <instinct/CoreGlobals.hpp>


namespace INSTINCT_CORE_NS {

class HttpClientException final: public InstinctException {
public:
    const unsigned int status_code_;
    const std::string raw_response_;
    std::string partial_msg_;


    HttpClientException(const unsigned int status_code, std::string raw_response): InstinctException(""), status_code_(status_code), raw_response_(std::move(raw_response)) {
        partial_msg_ = fmt::format("status_code={}, raw_response={} {}",  status_code_, raw_response_, InstinctException::message());
    }

    HttpClientException(const std::string& basic_string, const unsigned int status_code, std::string raw_response)
        : InstinctException(basic_string),
          status_code_(status_code),
          raw_response_(std::move(raw_response)) {
        partial_msg_ = fmt::format("status_code={}, raw_response={} {}",  status_code_, raw_response_, InstinctException::message());
    }

    const char* message() const noexcept override {
        return partial_msg_.data();
    }
};

} // namespace INSTINCT_CORE_NS

#endif //HTTPCLIENTEXCEPTION_H
