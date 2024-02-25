//
// Created by RobinQu on 2024/2/10.
//


#include <gtest/gtest.h>

#include "tools/SimpleHttpClient.hpp"
#include <ranges>

using namespace langchain::core;

TEST(SimpleHTTPCientTest, TestStream) {

    class foo {
        std::vector<int> data_;

    public:
        auto begin() {
            return data_.begin();
        }

        auto end() {
            return data_.end();
        }
    };


    static_assert(std::input_iterator<HttpChunkStream<std::string>::iterator>, "failed input iterator");
    static_assert(std::ranges::input_range<HttpChunkStream<std::string>>, "failed as input range");

    auto request_str = R"({
        "model": "llama2",
        "prompt": "Why is the sky blue?"
        }
    )";
    SimpleHttpClient client {{"localhost", 11434}};
    HttpRequest request = {POST, "/api/generate", {}, request_str};

    auto string_view = client.Stream(request);
    const auto chunk_range = string_view | std::views::transform([](const std::string& v) {
        return "response : " + v;
    });
    for(const auto& chunk: chunk_range) {
        std::cout << chunk << std::endl;
    }


}