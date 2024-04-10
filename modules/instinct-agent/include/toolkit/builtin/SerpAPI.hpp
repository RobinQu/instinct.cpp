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
        FunctionToolOptions base_options = {};
        Endpoint endpoint = SERP_API_DEFAULT_ENDPOINT;
        std::string apikey;
        std::string engine = "google";
    };

    class SerpAPI final: public BaseSearchTool {
        CURLHttpClient client_;
        SerpAPIOptions options_;
    public:
        explicit SerpAPI(const SerpAPIOptions& options): BaseSearchTool(options.base_options), client_(), options_(options) {
        }

        SearchToolResponse DoExecute(const SearchToolRequest &input) override {
            assert_true(StringUtils::IsNotBlankString(input.query()), "should provide non-blank query");

            // offset defaults to zero
            const size_t start = input.result_offset() <= 0 ? 0 : input.result_offset();
            // limit defaults to 3
            const size_t limit = input.result_limit() <= 0 ? 3 : input.result_limit();

            const HttpRequest call {
                .endpoint = options_.endpoint,
                .target = "/search",
                .paramters = {
                    {"q", input.query()},
                    {"engine", options_.engine},
                    {"api_key", options_.apikey},
                    {"start", std::to_string(start)},
                    {"num", std::to_string(limit)}
                }
            };

            const auto& [headers, body, status_code] = client_.Execute(call);
            assert_true(status_code == 200, "SERP API should have successful response.");
            LOG_DEBUG("raw response body: {}", body);
            const auto serp_response = ProtobufUtils::Deserialize<SerpAPISearchResponse>(body);
            SearchToolResponse search_tool_response;

            // use answer box
            if (serp_response.has_answer_box()) {
                auto* entry = search_tool_response.add_entries();
                entry->set_content(serp_response.answer_box().result());
            }

            // use QA results
            for (int i=0,k=search_tool_response.entries_size(); i<limit-k && i<serp_response.questions_and_answers_size(); ++i) {
                const auto& qa_result = serp_response.questions_and_answers(i);
                auto* entry = search_tool_response.add_entries();
                entry->set_content(qa_result.answer());
                entry->set_title(qa_result.title());
                entry->set_url(qa_result.link());
            }

            // and then organic results
            for(int i=0,k=search_tool_response.entries_size(); i<limit-k && i<serp_response.organic_results_size(); ++i) {
                const auto& result = serp_response.organic_results(i);
                auto* entry = search_tool_response.add_entries();
                entry->set_content(result.snippet());
                entry->set_title(result.title());
                entry->set_url(result.link());
            }
            return search_tool_response;
        }

         FunctionToolSelfCheckResponse SelfCheck() override {
            FunctionToolSelfCheckResponse response;
            SearchToolRequest search_tool_request;
            search_tool_request.set_query("cat");
            const auto search_tool_resp = DoExecute(search_tool_request);
            response.set_passed(search_tool_resp.entries_size() > 0);
            return response;
        }
    };

    static FunctionToolPtr CreateSerpAPI(const SerpAPIOptions& options) {
        return std::make_shared<SerpAPI>(options);
    }
}


#endif //SERPAPI_HPP
