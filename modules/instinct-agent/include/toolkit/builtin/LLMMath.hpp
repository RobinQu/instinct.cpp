//
// Created by RobinQu on 2024/4/9.
//

#ifndef LLMMATH_HPP
#define LLMMATH_HPP

#include <agent.pb.h>

#include <utility>

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
                prompt_template_ = CreatePlainPromptTemplate(R""(Translate a math problem into a expression that can be evaluated by a math library. Use the output of running this code to answer the question.

Question: ${{Question with math problem.}}
```text
${{single line mathematical expression that solves the problem}}
```
...evaluate(text)...
```output
${{Output of running the code}}
```
Answer: ${{Answer}}

Begin.

Question: What is 37593 * 67?
```text
37593 * 67
```
...evaluate("37593 * 67")...
```output
2518731
```
Answer: 2518731

Question: 37593^(1/5)
```text
37593**(1/5)
```
...evaluate("37593**(1/5)")...
```output
8.222831614237718
```
Answer: 8.222831614237718

Question: {question})"");
            }

            chat_model_->Configure({.stop_words = {"```output"}});

            chain_ = CreateTextChain(chat_model_, prompt_template_);
        }

        CalculatorToolResponse DoExecute(const CaculatorToolRequest &input) override {
            static std::string ANSWER_TOKENS = "Answer:";
            static std::string TEXT_TOKENS = "```text";
            static auto MATH_EXPRESSION_REGEX = std::regex{"^```text(.*?)```"};
            auto answer_string = chain_->Invoke(input.math_question());
            CalculatorToolResponse response;
            if (answer_string.starts_with(ANSWER_TOKENS)) {
                // model gives answer directly
                response.set_value(answer_string);
            } else if (const auto idx = answer_string.find(ANSWER_TOKENS); idx != std::string::npos) {
                // model output contains answer
                response.set_value(ANSWER_TOKENS + answer_string.substr(idx));
            } else if (std::smatch text_match; std::regex_match(answer_string, text_match, MATH_EXPRESSION_REGEX) && text_match.size() == 2) {
                // no answer is generated, let's extract math expression and run with math lib
                response.set_value(ANSWER_TOKENS + text_match[1].str());
            } else {
                LOG_DEBUG("model output: {}", answer_string);
                throw InstinctException("Malformed output for LLMMath");
            }
            return response;
        }
    };
}


#endif //LLMMATH_HPP
