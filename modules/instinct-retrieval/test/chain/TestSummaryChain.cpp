//
// Created by RobinQu on 2024/6/3.
//
#include <gtest/gtest.h>

#include "RetrieverObjectFactory.hpp"
#include "chain/SummaryChain.hpp"
#include "chat_model/OpenAIChat.hpp"
#include "ingestor/DirectoryTreeIngestor.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    class SummaryChainTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
        }

        std::filesystem::path corpus_dir = std::filesystem::current_path() / "_corpus";
        ChatModelPtr chat_model_ = CreateOpenAIChatModel();
    };


    TEST_F(SummaryChainTest, DoSummaryWithString) {
        const auto chain = CreateSummaryChain(chat_model_);
        const auto text1 = chain->Invoke({"The Donner Party was a group of American pioneers who set out for California in a wagon train, but became snowbound in the Sierra Nevada mountains in November 1846. Running out of food, some resorted to cannibalism to survive. The journey west usually took between four and six months, but the Donner Party had been slowed by following a new route called the Hastings Cutoff, which crossed the Rocky Mountains' Wasatch Range and the Great Salt Lake Desert in present-day Utah. They lost many cattle and wagons in the rugged terrain, and divisions formed within the group. Their food supplies ran low after they became trapped by an early, heavy snowfall high in the mountains. In mid-December some of the group set out on foot and were able to obtain help. Of the 87 members of the party, 48 survived to reach California. Historians have described the episode as one of the most spectacular tragedies in California history."});

        ASSERT_FALSE(text1.empty());
        ASSERT_TRUE(text1.find("Donner") != std::string::npos);
    }

    TEST_F(SummaryChainTest, DoSummaryWithParagraphs) {
        const auto ingestor = RetrieverObjectFactory::CreateDirectoryTreeIngestor(
            corpus_dir / "recipes"
        );
        auto is_reducible = [](const std::vector<std::string>& data) {
            size_t total = 0;
            for(const auto& item: data) {
                total += item.size();
            }
            return total >= 16 * 1024;
        };
        const auto chain = CreateSummaryChain(chat_model_);
        const auto itr = ingestor->Load() |  rpp::ops::map([](const Document& doc) {return doc.text();});
        auto f1 = CreateSummary(itr, chain, is_reducible);
        LOG_INFO("f1 = {}", f1.get());
    }
}
