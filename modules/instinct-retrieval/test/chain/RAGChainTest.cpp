//
// Created by RobinQu on 2024/3/11.
//

#include <gtest/gtest.h>

#include "RetrievalGlobals.hpp"
#include "chain/RAGChain.hpp"
#include "chat_model/OllamaChat.hpp"
#include "embedding_model/OllamaEmbedding.hpp"
#include "memory/EphemeralChatMemory.hpp"
#include "retrieval/VectorStoreRetriever.hpp"
#include "tools/ChronoUtils.hpp"
#include "store/duckdb/DuckDBVectorStore.hpp"
#include "LLMTestGlobals.hpp"

namespace  INSTINCT_RETRIEVAL_NS {
    class RAGChainTest : public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();

            embedding_model_ = instinct::test::create_local_embedding_model();
            const auto db_file_path = std::filesystem::temp_directory_path() / (
                                    ChronoUtils::GetCurrentTimestampString() + ".db");

            auto vector_store = CreateDuckDBVectorStore(embedding_model_, {
                .table_name = "rag_test_table",
                .db_file_path = db_file_path,
                // llama2:7b-chat has dimensions of 4096
                .dimension = 4096
            });
            retriever_ = CreateVectorStoreRetriever(vector_store);
            chat_memory_ = std::make_shared<EphemeralChatMemory>();

            ChatModelPtr chat_model = instinct::test::create_local_chat_model();

            PromptTemplatePtr question_prompt_template = OllamaChat::CreateChatPromptTemplateBuilder()
                    ->AddHumanMessage(R"(
Given the following conversation and a follow up question, rephrase the follow up question to be a standalone question, in its original language.
Chat History:
{chat_history}
Follow Up Input: {question}
Standalone question:)")
            ->Build();


            ChainOptions question_chain_options = {
//                .input_keys = {"question"},
//                .output_keys = {"standalone_question"}
            };

            question_chain_ = CreateTextChain(
                chat_model,
                question_prompt_template,
                question_chain_options
            );

            PromptTemplatePtr answer_prompt_template = OllamaChat::CreateChatPromptTemplateBuilder()
            ->AddHumanMessage(R"(Answer the question based only on the following context:
{context}

Question: {standalone_question}

{format_instruction}
)")
            ->Build();

            ChainOptions answer_chain_options = {
//                .input_keys  = {"standalone_question", "context"},
//                .output_keys = {"answer"}
            };
            answer_chain_ = CreateTextChain(
                chat_model,
                answer_prompt_template,
                answer_chain_options
            );

            RAGChainOptions rag_chain_options = {
//                .context_output_key = "context",
//                .condense_question_key = "standalone_question",
            };
            rag_chain_ = CreateTextRAGChain(
                retriever_,
                chat_model,
                chat_memory_,
                question_prompt_template,
                answer_prompt_template,
                rag_chain_options
                );
        }

        EmbeddingsPtr embedding_model_;
        StatefulRetrieverPtr retriever_;
        ChatMemoryPtr chat_memory_;
        TextChainPtr question_chain_;
        TextChainPtr answer_chain_;
        TextChainPtr rag_chain_;
    };

    TEST_F(RAGChainTest, SimpleQAChat) {
        // run with empty docs
        auto output = rag_chain_->Invoke("why sea is blue?");
        std::cout << "output = " << output << std::endl;

        // invoke again to verify chat_history
        output = rag_chain_->Invoke("Can you explain in a way that even 6-year child could understand?");
        std::cout << "output = " << output << std::endl;

        // create a new context and verify
        auto message_list = chat_memory_->LoadMemories();
        ASSERT_TRUE(message_list.messages_size()>0);
    }
}
