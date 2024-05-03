//
// Created by RobinQu on 2024/4/30.
//
#include <gtest/gtest.h>
#include "LLMGlobals.hpp"
#include "agent/patterns/openai_tool/Agent.hpp"
#include "agent/executor/BaseAgentExecutor.hpp"
#include "chat_model/OpenAIChat.hpp"
#include "toolkit/LocalToolkit.hpp"

namespace INSTINCT_LLM_NS {
    class OpenAIToolAgentTest: public testing::Test {
        class GetNightlyHotelPrice final: public BaseFunctionTool {
            FunctionTool schema_;

        public:
            explicit GetNightlyHotelPrice(const FunctionToolOptions &options = {})
                : BaseFunctionTool(options) {
                ProtobufUtils::Deserialize(R"(
{
    "name": "get_nightly_hotel_price",
    "description": "Get hotel room price for a given city",
    "arguments": [
        {
            "name" :"city",
            "type": "string",
            "description": "The city to get hotel prices for",
            "required": true
        }
    ]
}
)", schema_);
            }

            [[nodiscard]] const FunctionTool & GetSchema() const override {
                return schema_;
            }

            std::string Execute(const std::string &action_input) override {
                static std::unordered_map<std::string, float> price_maps {
                    {"New York", 300},
                    {"Paris", 200},
                    {"Tokyo", 300}
                };
                const auto args = nlohmann::json::parse(action_input);
                const auto city = args.at("city").get<std::string>();
                nlohmann::json resp;
                resp["city"] = city;
                resp["hotel_price"] = price_maps.at(city);
                return resp.dump();
            }
        };

        class GetFlightPriceTool final: public BaseFunctionTool {
            FunctionTool schema_;

        public:
            explicit GetFlightPriceTool(const FunctionToolOptions &options = {})
                : BaseFunctionTool(options) {
                ProtobufUtils::Deserialize(R"(
{
    "name": "get_flight_price",
    "description": "Get flight price for a given city",
    "arguments": [
        {
            "name" :"city",
            "type": "string",
            "description": "The city to get flight prices for",
            "required": true
        }
    ]
}
)", schema_);
            }

            [[nodiscard]] const FunctionTool & GetSchema() const override {
                return schema_;
            }

            std::string Execute(const std::string &action_input) override {
                static std::unordered_map<std::string, float> price_maps {
                            {"New York", 450},
                            {"Paris", 750},
                            {"Tokyo", 1200}
                };
                const auto args = nlohmann::json::parse(action_input);
                const auto city = args.at("city").get<std::string>();
                nlohmann::json resp;
                resp["city"] = city;
                resp["flight_price"] = price_maps.at(city);
                return resp.dump();
            }
        };


    protected:
        void SetUp() override {
            SetupLogging();
        }

        ChatModelPtr chat_model = CreateOpenAIChatModel();
        FunctionToolkitPtr tool_kit = CreateLocalToolkit(
            std::make_shared<GetFlightPriceTool>(),
            std::make_shared<GetNightlyHotelPrice>()
        );
    };


    /**
     * A use case extracted from: https://gist.github.com/gaborcselle/5d9e45232319b543ca5336f8ab23e8d2
     **/
    TEST_F(OpenAIToolAgentTest, NormalExecution) {
        const auto agent_executor = CreateOpenAIToolAgentExecutor(chat_model, {tool_kit});
        const auto initial_state = agent_executor->InitializeState("How much would a 3 day trip to New York, Paris, and Tokyo cost?");
        agent_executor->Stream(initial_state)
            | rpp::operators::as_blocking()
            | rpp::operators::subscribe([](const AgentState& state) {
                auto& latest_step = *state.previous_steps().rbegin();
                if (latest_step.thought().has_finish()) {
                    LOG_INFO("Final answer: {}", latest_step.thought().finish().response());
                }
            })
        ;
    }
}
