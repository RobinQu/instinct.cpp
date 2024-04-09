//
// Created by RobinQu on 2024/4/9.
//


#include <gtest/gtest.h>

#include "chat_model/OllamaChat.hpp"
#include "toolkit/builtin/LLMMath.hpp"

namespace INSTINCT_AGENT_NS {

    class TestLLMMath: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
            const ChatModelPtr chat_model = CreateOllamaChatModel();
            math = CreateLLMMath(chat_model);
        }

        [[nodiscard]] std::string Calculate(const std::string& question) const {
            CaculatorToolRequest request;
            request.set_math_question(question);
            FunctionToolInvocation invocation;
            invocation.set_input(ProtobufUtils::Serialize(request));
            const auto resp = math->Invoke(invocation);
            const auto calc_resp = ProtobufUtils::Deserialize<CalculatorToolResponse>(resp.return_value());
            return calc_resp.value();
        }

        FunctionToolPtr math;
    };

    TEST_F(TestLLMMath, MatchExpression) {
        const std::string resp = R"(Sure, I'd be happy to help! Here is the math expression for the problem you provided:

Question: What's the result of three plus 4 multiplied by 0.5?
Math expression: 3 + 4 * 0.5)";
        std::string pattern_text = R"(Math expression:\s*(.+))";
        const auto expression_regex = std::regex(pattern_text);
        std::smatch text_match;
        std::regex_match(resp, text_match, expression_regex);
        ASSERT_EQ(text_match.size(), 2);
        for(const auto& match: text_match) {
            LOG_INFO("match: {}", match.str());
        }
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
        T result = parser.compile("3 + 4 * 0.5" ,symbol_table);
        LOG_INFO("result = {}", result);
        ASSERT_EQ(result, 5);
    }

    TEST_F(TestLLMMath, SimpleCalculation) {
        auto r1 = Calculate("What's  result of three plus 4 multiplied by 0.5?");
        ASSERT_EQ(r1, "3.5");
    }
}