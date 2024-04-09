//
// Created by RobinQu on 2024/4/9.
//

#ifndef SERPAPI_HPP
#define SERPAPI_HPP

#include <agent.pb.h>

#include <utility>

#include "toolkit/BaseSearchTool.hpp"
#include "tools/HttpRestClient.hpp"



namespace INSTINCT_AGENT_NS {
    using namespace INSTINCT_CORE_NS;

    static Endpoint SERP_API_DEFAULT_ENDPOINT {
        .protocol = kHTTPS,
        .host = "serpapi.com",
        .port = 443
    };

    struct SerpAPIOptions {
        Endpoint endpoint = SERP_API_DEFAULT_ENDPOINT;
        std::string apikey;
        std::string engine = "google";
    };

    class SerpAPI final: BaseSearchTool {
         CURLHttpClient client_;
        SerpAPIOptions options_;
    public:
        explicit SerpAPI(SerpAPIOptions  options): client_(), options_(std::move(options)) {
        }

        SearchToolResponse DoExecute(const SearchToolRequest &input) override {
            const std::string path_string = fmt::format("/search?q={}&engine={}&api_key={}&start={}&num={}",
                input.query(),
                options_.engine,
                options_.apikey,
                input.result_offset(),
                input.result_limit()
                );
            const auto& [headers, body, status_code] = client_.Execute({.endpoint = options_.endpoint, .target = path_string});
            assert_true(status_code == 200, "SERP API should have successful response.");
            const auto serp_response = ProtobufUtils::Deserialize<SerpAPISearchResponse>(body);
            SearchToolResponse search_tool_response;
            for(const auto& result: serp_response.organic_results()) {
                auto* entry = search_tool_response.add_entries();
                entry->set_content(result.snippet());
                entry->set_title(result.title());
                entry->set_url(result.link());
            }
            return search_tool_response;
        }
    };
}


#endif //SERPAPI_HPP
