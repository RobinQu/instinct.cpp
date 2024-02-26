//
// Created by RobinQu on 2024/2/10.
//


#include <gtest/gtest.h>

#include "tools/SimpleHttpClient.hpp"
#include <ranges>


using namespace langchain::core;

LC_CORE_NS {
    TEST(SimpleHTTPCientTest, TestStream) {
        auto request_str = R"({
            "model": "llama2",
            "prompt": "Why is the sky blue?"
        }
        )";
        SimpleHttpClient client{{"localhost", 11434}};
        HttpRequest request = {POST, "/api/generate", {}, request_str};

        auto* chunk_view = client.Stream(request);
        while (chunk_view->HasNext()) {
            std::cout << chunk_view->Next() << std::endl;
        }
        delete chunk_view;
    }
}
