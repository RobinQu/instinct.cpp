//
// Created by RobinQu on 2024/2/10.
//


#include <gtest/gtest.h>

#include "tools/SimpleHttpClient.hpp"
#include <ranges>


using namespace INSTINCT_CORE_NS;

namespace INSTINCT_CORE_NS {
    TEST(SimpleHTTPCientTest, TestStream) {
        auto request_str = R"({
            "model": "llama2",
            "prompt": "Why is the sky blue?",
            "stream": true
        }
        )";
        SimpleHttpClient client{{.host = "localhost", .port=11434}};
        HttpRequest request = {kPOST, "/api/generate", {}, request_str};

        auto chunk_view = client.StreamChunk(request);
        chunk_view | rpp::operators::subscribe([](const auto& chunk) {
                std::cout << chunk << std::endl;
            });
    }
}
