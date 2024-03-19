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
            SetupLogging();
            string_parser_ = std::make_shared<StringOutputParser>();
            chat_model_ = std::make_shared<OllamaChat>();
            llm_ = std::make_shared<OllamaLLM>();

            auto builder = OllamaChat::CreateChatPromptTemplateBuilder();
            builder->AddHumanMessage("why is the sky blue?");
            builder->AddAIMessage("due to rayleigh scattering.");
            builder->AddHumanMessage("{question}");
            chat_prompt_template_ = builder->Build();
            string_prompt_template_ = PlainPromptTemplate::CreateWithTemplate(" {question}");
        }
        ChatModelPtr chat_model_;
        LLMPtr llm_;
        PromptTemplatePtr chat_prompt_template_;
        PromptTemplatePtr string_prompt_template_;
        OutputParserPtr<std::string> string_parser_;
    };


    TEST_F(LLMChainTest, GenerateWithLLM) {
        LLMChain chain { llm_, string_prompt_template_, string_parser_, nullptr};
        LLMChainContext context;
        auto builder = ContextMutataor::Create();
        builder->Put("question", "Why is sky blue?");
        auto result = chain.Invoke(builder->Build());
        std::cout << result << std::endl;

        chain.Batch({builder->Build()})
            | rpp::operators::subscribe([](const auto& msg) { LOG_INFO("msg={}", msg); });

        auto chunk_itr = chain.Stream(builder->Build());
        std::string buf;
        for (const auto& chunk_result: CollectVector(chunk_itr)) {
            buf += chunk_result;
            std::cout << buf << std::endl;;
        }
    }

    TEST_F(LLMChainTest, GenerateWithChatModel) {
        auto builder = ContextMutataor::Create();
        builder->Put("question", "how is that different than mie scattering?");
        LLMChain chain {chat_model_, chat_prompt_template_, string_parser_, nullptr};
        auto result = chain.Invoke(builder->Build());
        std::cout << result << std::endl;


        chain.Batch({builder->Build()})
            | rpp::operators::subscribe([](const auto& msg) { LOG_INFO("msg={}", msg); });

        auto chunk_itr = chain.Stream(builder->Build());
        std::string buf;
        for (const auto& chunk_result: CollectVector(chunk_itr)) {
            buf += chunk_result;
            std::cout << buf << std::endl;;
        }

    }

}
