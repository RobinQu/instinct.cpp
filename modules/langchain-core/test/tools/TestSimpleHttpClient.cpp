//
// Created by RobinQu on 2024/2/10.
//


#include <gtest/gtest.h>

#include "tools/SimpleHttpClient.h"

using namespace langchain::core;

TEST(SimpleHTTPCientTest, TestStream) {

    auto request_str = R"({
        "model": "llama2",
        "prompt": "Why is the sky blue?"
        }
    )";
    SimpleHttpClient client {{"localhost", 11434}};
    HttpRequest request = {POST, "/api/generate", {}, request_str};
    const auto itr = client.Stream(request);
    while (itr->HasNext()) {
        std::cout << itr->Next() << std::endl;
    }

}