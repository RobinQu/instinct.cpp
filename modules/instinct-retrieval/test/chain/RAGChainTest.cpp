//
// Created by RobinQu on 2024/3/11.
//

#include <gtest/gtest.h>

#include <instinct/RetrievalGlobals.hpp>
#include <instinct/chain/RAGChain.hpp>
#include <instinct/chat_model/ollama_chat.hpp>
#include <instinct/embedding_model/ollama_embedding.hpp>
#include <instinct/memory/ephemeral_chat_memory.hpp>
#include <instinct/retrieval/VectorStoreRetriever.hpp>
#include <instinct/tools/chrono_utils.hpp>
#include <instinct/store/duckdb/DuckDBVectorStore.hpp>
#include <instinct/llm_test_global.hpp>
#include <instinct/prompt/plain_chat_prompt_template.hpp>

namespace  INSTINCT_RETRIEVAL_NS {
    class RAGChainTest : public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();

            size_t dim = 4096;
            embedding_model_ = create_pesudo_embedding_model(dim);
            const auto db_file_path = std::filesystem::temp_directory_path() / (
                                    ChronoUtils::GetCurrentTimestampString() + ".db");

            const auto vector_store = CreateDuckDBVectorStore(embedding_model_, {
                .table_name = "rag_test_table",
                .db_file_path = db_file_path,
                // llama2:7b-chat has dimensions of 4096
                .dimension = dim
            });
            retriever_ = CreateVectorStoreRetriever(vector_store);
            chat_memory_ = std::make_shared<EphemeralChatMemory>();

            ChatModelPtr chat_model = create_pesudo_chat_model();

            const auto question_prompt_template =
                CreatePlainChatPromptTemplate({
                    {kHuman, R"(
Given the following conversation and a follow up question, rephrase the follow up question to be a standalone question, in its original language.
Chat History:
{chat_history}
Follow Up Input: {question}
Standalone question:)"}
                });

            ChainOptions question_chain_options = {
//                .input_keys = {"question", "chat_history"},
//                .output_keys = {"standalone_question"}
            };

            question_chain_ = CreateTextChain(
                chat_model,
                question_prompt_template,
                question_chain_options
            );

            const auto answer_prompt_template =
                CreatePlainChatPromptTemplate({
                        {kHuman, R"(Answer the question based only on the following context:
{context}

Question: {standalone_question}
)"}
                });

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
