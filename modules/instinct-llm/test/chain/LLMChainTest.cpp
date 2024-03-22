//
// Created by RobinQu on 2024/3/10.
//

#include <gtest/gtest.h>

#include "LLMGlobals.hpp"
#include "chain/LLMChain.hpp"
#include "chat_model/OllamaChat.hpp"
#include "llm/OllamaLLM.hpp"
#include "output_parser/GenerationOutputParser.hpp"
#include "prompt/PlainChatTemplate.hpp"
#include "prompt/PlainPromptTemplate.hpp"


namespace INSTINCT_LLM_NS {


    class LLMChainTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
//            input_parser_ = std::make_shared<PromptValueInputParser>();
            output_parser_ = std::make_shared<GenerationOutputParser>();
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
//        InputParserPtr<PromptValue> input_parser_;
        OutputParserPtr<Generation> output_parser_;
    };


    TEST_F(LLMChainTest, GenerateWithLLM) {
        TextChain chain  {
            string_prompt_template_,
            llm_,
            output_parser_,
            nullptr,
            {}
        };

        auto pv = CreateJSONContext({"question", "Why sky is blue?"});
        auto result = chain.Invoke(pv);
        std::cout << result.DebugString() << std::endl;

        chain.Batch({pv})
            | rpp::operators::subscribe([](const auto& msg) { LOG_INFO("msg={}", msg.DebugString()); });

        auto chunk_itr = chain.Stream(pv);
        std::string buf;
        for (const auto& chunk_result: CollectVector(chunk_itr)) {
            buf += chunk_result.text();
            std::cout << buf << std::endl;;
        }
    }

    TEST_F(LLMChainTest, GenerateWithChatModel) {
        auto pv = CreateJSONContext({"question", "how is that different than mie scattering?"});
        TextChain chain { chat_prompt_template_, chat_model_, output_parser_, nullptr, {}};
        auto result = chain.Invoke(pv);
        std::cout << result.DebugString() << std::endl;

        chain.Batch({pv})
            | rpp::operators::subscribe([](const auto& msg) { LOG_INFO("msg={}", msg.DebugString()); });

        auto chunk_itr = chain.Stream(pv);
        std::string buf;
        for (const auto& chunk_result: CollectVector(chunk_itr)) {
            buf += chunk_result.message().content();
            std::cout << buf << std::endl;;
        }
    }

}
