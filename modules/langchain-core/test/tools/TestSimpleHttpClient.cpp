//
// Created by RobinQu on 2024/2/10.
//


#include <gtest/gtest.h>

#include "tools/SimpleHttpClient.hpp"
#include <ranges>

#include "tools/ChunkStreamView.hpp"

using namespace langchain::core;

LC_CORE_NS {


    auto create_csv_from_ollama(SimpleHttpClient& client, const HttpRequest& request) {
        return wrap_chunk_stream_view(client.Stream(request));
    }

    TEST(SimpleHTTPCientTest, TestStream) {
        static_assert(std::input_iterator<HttpChunkStream<std::string>::iterator>, "failed input iterator");
        static_assert(std::ranges::input_range<HttpChunkStream<std::string>>, "failed as input range");

        auto request_str = R"({
        "model": "llama2",
        "prompt": "Why is the sky blue?"
        }
    )";
        SimpleHttpClient client{{"localhost", 11434}};
        HttpRequest request = {POST, "/api/generate", {}, request_str};

        // auto string_view = client.Stream(request);
        // const auto chunk_range = string_view | std::views::transform([](const std::string& v) {
        //     return "response : " + v;
        // });
        //
        // for (const auto& chunk: chunk_range) {
        //     std::cout << chunk << std::endl;
        // }

        auto chunk_view = create_csv_from_ollama(client, request);
        for (const auto& chunk: chunk_view) {
            std::cout << chunk << std::endl;
        }
    }
}
