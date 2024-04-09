//
// Created by RobinQu on 2024/4/9.
//

#ifndef LLMMATH_HPP
#define LLMMATH_HPP

#include <agent.pb.h>
#include <exprtk.hpp>

#include "chain/LLMChain.hpp"
#include "chat_model/BaseChatModel.hpp"
#include "prompt/PlainChatPromptTemplate.hpp"
#include "prompt/PlainPromptTemplate.hpp"
#include "toolkit/ProtoMessageFunctionTool.hpp"



namespace INSTINCT_AGENT_NS {
    class LLMMath final: public ProtoMessageFunctionTool<CaculatorToolRequest, CalculatorToolResponse> {
        ChatModelPtr chat_model_;
        PromptTemplatePtr prompt_template_;
        TextChainPtr chain_;

    public:
        explicit LLMMath(ChatModelPtr chat_model, PromptTemplatePtr prompt_template = nullptr)
            : chat_model_(std::move(chat_model)),
              prompt_template_(std::move(prompt_template)) {
            if(!prompt_template_) {
                prompt_template_ = CreatePlainPromptTemplate(R""(
Translate a math problem into a expression that can be evaluated by a math library. Please don't calculate result directly.

Use the following format:

Question: What is 37593 * 67?
Math expression: 37593 * 67

Question: 37593^(1/5)
Math expression: 37593**(1/5)

Begin!

Question: {question}
Math expression:)"");
            }
            chain_ = CreateTextChain(chat_model_, prompt_template_);
        }

        CalculatorToolResponse DoExecute(const CaculatorToolRequest &input) override {
            static auto MATH_EXPRESSION_REGEX = std::regex{R"(Math expression:\s*(.*)$)"};
            auto answer_string = chain_->Invoke(input.math_question());
            LOG_DEBUG("model output: {}", answer_string);
            CalculatorToolResponse response;
            if (std::smatch text_match; std::regex_match(answer_string, text_match, MATH_EXPRESSION_REGEX) && text_match.size() == 2) {
                const auto v = Evaulate_<double>(text_match[1].str());
                response.set_value(std::to_string(v));
            } else {
                throw InstinctException("Malformed output for LLMMath");
            }
            return response;
        }


    private:
        template <typename T>
        T Evaulate_(const std::string& expression_string) {
            typedef exprtk::symbol_table<T> symbol_table_t;
            typedef exprtk::expression<T>   expression_t;
            typedef exprtk::parser<T>       parser_t;
            symbol_table_t symbol_table;
            parser_t parser;
            return parser.compile(expression_string,symbol_table);
        }
    };


    static FunctionToolPtr CreateLLMMath(const ChatModelPtr& chat_model, const PromptTemplatePtr& prompt_template = nullptr) {
        return std::make_shared<LLMMath>(chat_model, prompt_template);
    }
}


#endif //LLMMATH_HPP
