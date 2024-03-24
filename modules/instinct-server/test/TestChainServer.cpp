//
// Created by RobinQu on 3/19/24.
//
#include <gtest/gtest.h>
#include "http/MultiChainServer.hpp"
#include "chain/LLMChain.hpp"
#include "chat_model/OpenAIChat.hpp"
#include "LLMTestGlobals.hpp"

namespace INSTINCT_SERVER_NS {
    using namespace INSTINCT_LLM_NS;
    using namespace INSTINCT_CORE_NS;

    class PlainChainServerTest : public ::testing::Test {

    protected:
        void SetUp() override {
            SetupLogging();
            auto llm = CreateOpenAIChatModel(test::DEFAULT_NITRO_SERVER_CONFIGURATION);

            chain1_ = CreatePassthroughChain(llm);
        }
        PassthroughChainPtr chain1_;

    };


    TEST_F(PlainChainServerTest, Lifecycle) {
        MultiChainServer server;
        server.AddNamedChain("chain1", chain1_);
        server.Start();
        server.Shutdown();
    }
}

