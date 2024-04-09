//
// Created by RobinQu on 2024/4/9.
//

#ifndef AGENTTESTGLOBALS_HPP
#define AGENTTESTGLOBALS_HPP


#include <random>

#include "AgentGlobals.hpp"
#include "toolkit/BaseSearchTool.hpp"
#include "toolkit/LambdaFunctionTool.hpp"
#include "toolkit/ProtoMessageFunctionTool.hpp"

namespace INSTINCT_AGENT_NS::test {

    class MockSearchTool final: public BaseSearchTool {
        std::multimap<std::string, SearchToolResponseEntry> entries_;
    public:
        void AddResponse(const std::string& q, const std::string& content, std::string title = "", std::string link = "") {
            SearchToolResponseEntry entry;
            entry.set_title(title);
            entry.set_content(content);
            if (StringUtils::IsBlankString(link)) {
                link = "https://google.com/search?q=" + q;
            }
            if (StringUtils::IsBlankString(title)) {
                title = "Search result about " + q;
            }
            entry.set_url(link);
            entries_.emplace(q, entry);
        }

        SearchToolResponse DoExecute(const SearchToolRequest &input) override {
            SearchToolResponse response;
            if (entries_.contains(input.query())) {
                // exact match
                for(const auto& entry: entries_.equal_range(input.query())) {
                    response.add_entries()->CopyFrom(entry);
                }
            } else {
                // give random result
                std::random_device r;
                // Choose a random mean between 1 and 6
                std::default_random_engine e1(r());
                std::uniform_int_distribution<int> uniform_dist(0, entries_.size());
                auto itr = entries_.begin();
                for(int i=0;i<uniform_dist(e1);++i) {
                    ++itr;
                }
                response.add_entries()->CopyFrom(itr->second);
            }
            return response;
        }
    };

}


#endif //AGENTTESTGLOBALS_HPP
