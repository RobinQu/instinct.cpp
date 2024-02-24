//
// Created by RobinQu on 2024/2/10.
//


#include <gtest/gtest.h>

#include "tools/SimpleHttpClient.hpp"

using namespace langchain::core;

TEST(SimpleHTTPCientTest, TestStream) {

    auto request_str = R"({
        "model": "llama2",
        "prompt": "Why is the sky blue?"
        }
    )";
    SimpleHttpClient client {{"localhost", 11434}};
    HttpRequest request = {POST, "/api/generate", {}, request_str};
    auto stream = client.Stream(request);
    for(auto& chunk: stream) {
        std::cout << chunk << std::endl;
    }


}