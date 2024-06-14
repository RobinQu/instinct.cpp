//
// Created by RobinQu on 2024/4/9.
//


#include <gtest/gtest.h>

#include "chat_model/OpenAIChat.hpp"
#include "toolkit/builtin/LLMMath.hpp"

namespace INSTINCT_AGENT_NS {
    using namespace INSTINCT_LLM_NS;

    class TestLLMMath: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
            const ChatModelPtr chat_model = CreateOpenAIChatModel({
                // gpt-3.5 often make mistakes
                .model_name = "gpt-4o",
                .temperature = 0
            });
            math = CreateLLMMath(chat_model, {.max_attempts = 3});
        }

        [[nodiscard]] std::string Calculate(const std::string& question) const {
            CaculatorToolRequest request;
            request.set_math_question(question);
            ToolCallObject invocation;
            invocation.mutable_function()->set_arguments(ProtobufUtils::Serialize(request));
            const auto resp = math->Invoke(invocation);
            // const auto calc_resp = ProtobufUtils::Deserialize<CalculatorToolResponse>(resp.return_value());
            // return calc_resp.value();
            return resp.return_value();
        }

        FunctionToolPtr math;
    };

    TEST_F(TestLLMMath, MatchExpression) {
        const std::string paragraph = R"(Sure, I can help you with that! The math problem you provided is:

Question: What's the result of three plus 4 multiplied by 0.5?
Math expression: 3 + 4 * 0.5

Can you please provide the math library you want to use for evaluation?)";

        const std::string pattern_text = R"(Math expression:\s*(.+))";
        const auto pattern = std::regex(pattern_text);
        std::sregex_iterator iter(paragraph.begin(), paragraph.end(), pattern);
        std::sregex_iterator end;

        std::vector<std::smatch> matches;

        while (iter != end) {
            matches.push_back(*iter++);
        }

        if (!matches.empty()) {
            for (const auto& match : matches) {
                for(size_t i=0; i<match.size(); ++i) {
                    std::cout << "Sub-match found: " << match.str(i) << '\n';
                }
            }
        } else {
            std::cout << "No match found.\n";
        }

        ASSERT_EQ(matches.size(), 1);
        ASSERT_EQ(matches[0].size(), 2);
        ASSERT_EQ(matches[0][1], "3 + 4 * 0.5");
    }

    TEST_F(TestLLMMath, EvaluateExpression) {
        using T = double;
        typedef exprtk::symbol_table<T> symbol_table_t;
        typedef exprtk::expression<T>   expression_t;
        typedef exprtk::parser<T>       parser_t;

        // static const T pi = T(3.141592653589793238462643383279502);
        symbol_table_t symbol_table;
        // expression_t expression;
        // expression.register_symbol_table(symbol_table);
        parser_t parser;
        T result = parser.compile("(3 + 4) * 0.5" ,symbol_table);
        LOG_INFO("result = {}", result);
        ASSERT_EQ(result, 3.5);
    }

    TEST_F(TestLLMMath, SelfTest) {
        const auto result = math->SelfCheck();
        ASSERT_TRUE(result.passed());
    }

    TEST_F(TestLLMMath, GetSchema) {
        const auto schema = math->GetSchema();
        LOG_INFO(">>> {}", schema.ShortDebugString());
        ASSERT_EQ(schema.parameters().properties_size(), 1);
        ASSERT_EQ(schema.parameters().properties().begin()->first, "math_question");
        ASSERT_EQ(schema.parameters().properties().begin()->second.type(), "string");
    }

    TEST_F(TestLLMMath, SimpleCalculation) {
        const auto r1 = Calculate("What's result of three plus 4 and then multiplied by 0.5?");
        LOG_INFO(">> {}", r1);
        ASSERT_EQ(r1, R"({"answer":3.5})");

        const auto r2 = Calculate("What's result of square root of 6?");
        LOG_INFO(">> {}", r2);
//        ASSERT_EQ(r2, R"({"answer":2.4494897427831779})");
    }
}