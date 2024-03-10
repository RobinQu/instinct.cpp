//
// Created by RobinQu on 2024/3/10.
//

#include <gtest/gtest.h>

#include "LLMGlobals.hpp"
#include "chain/LLMChain.hpp"
#include "chat_model/OllamaChat.hpp"
#include "llm/OllamaLLM.hpp"
#include "output_parser/StringOutputParser.hpp"
#include "prompt/PlainChatTemplate.hpp"
#include "prompt/PlainPromptTemplate.hpp"


namespace INSTINCT_LLM_NS {


    class LLMChainTest: public testing::Test {
    protected:
        void SetUp() override {
            string_parser_ = std::make_shared<StringOutputParser>();
            chat_model_ = std::make_shared<OllamaChat>();
            llm_ = std::make_shared<OllamaLLM>();

            auto builder = OllamaChat::CreateChatPromptTemplateBuilder();
            builder->AddSystemMessage("You are a help asistant. Please answer user's question in one sentence.");
            builder->AddHumanMessage("Question: {question}");
            chat_prompt_template_ = builder->Build();
            string_prompt_template_ = PlainPromptTemplate::CreateWithTemplate("Please answer following question in one sentence. \n Question: {question}");
        }
        ChatModelPtr chat_model_;
        LLMPtr llm_;
        PromptTemplatePtr chat_prompt_template_;
        PromptTemplatePtr string_prompt_template_;
        OutputParserPtr<std::string> string_parser_;
    };


    TEST_F(LLMChainTest, SimpleGenerate) {
        LLMChain chain { llm_, string_prompt_template_, string_parser_, nullptr};
        LLMChainContext context;

        PrimitiveVariable v;
        v.set_string_value("Why is sky blue?");
        v.set_name("question");
        context.mutable_values()
        ->emplace("question", v);
        const auto result = chain.Invoke(context);
        std::cout << result << std::endl;
    }

}
