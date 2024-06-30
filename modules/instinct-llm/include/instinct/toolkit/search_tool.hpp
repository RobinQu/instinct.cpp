//
// Created by RobinQu on 2024/4/9.
//

#ifndef BASESEARCHTOOL_HPP
#define BASESEARCHTOOL_HPP


#include <instinct/llm_global.hpp>
#include <instinct/toolkit/proto_message_function_tool.hpp>

namespace INSTINCT_LLM_NS {
    struct SearchToolOptions: FunctionToolOptions {
        bool format_response_entities = false;
    };

    class BaseSearchTool: public ProtoMessageFunctionTool<SearchToolRequest, SearchToolResponse> {
        SearchToolOptions options_;
    public:
        explicit BaseSearchTool(const SearchToolOptions &options, const std::string& name = "Search", const std::string& description = "A search engine. Useful for when you need to answer questions about current events. Input should be a search query.")
            : ProtoMessageFunctionTool(name, description, options), options_(options) {
        }

        std::string GetExample() override {
            return GetSchema().name() + R"(({"query":"keyword}))";
        }

        std::string Execute(const std::string &action_input) override {
            if (options_.format_response_entities) {
                auto input = ProtobufUtils::Deserialize<SearchToolRequest>(action_input);
                auto result = DoExecute(input);
                std::string output;
                for(const auto& item: result.entries()) {
                    output += item.title();
                    output += "\n";
                    output += item.content();
                    output += "\n";
                    output += fmt::format("Source: {}", item.url());
                    output += "\n\n";
                }
                return output;
            }
            return ProtoMessageFunctionTool::Execute(action_input);
        }
    };
}

#endif //BASESEARCHTOOL_HPP
