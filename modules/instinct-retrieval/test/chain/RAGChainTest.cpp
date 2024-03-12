//
// Created by RobinQu on 2024/3/11.
//

#include <gtest/gtest.h>

#include "RetrievalGlobals.hpp"
#include "chain/RAGChain.hpp"
#include "chat_model/OllamaChat.hpp"
#include "embedding_model/OllamaEmbedding.hpp"
#include "memory/EphemeralChatMemory.hpp"
#include "output_parser/StringOutputParser.hpp"
#include "retrieval/VectorStoreRetriever.hpp"
#include "tools/ChronoUtils.hpp"


namespace  INSTINCT_RETRIEVAL_NS {
    class RAGChainTest : public testing::Test {
    protected:
        void SetUp() override {
            OllamaConfiguration ollama_configuration = {.model_name = "llama2:7b-chat"};
            embedding_model_ = std::make_shared<OllamaEmbedding>(ollama_configuration);
            const auto db_file_path = std::filesystem::temp_directory_path() / (
                                    ChronoUtils::GetCurrentTimestampString() + ".db");

            retriever_ = VectorStoreRetriever::Create({
                .table_name = "rag_test_table",
                .db_file_path = db_file_path,
                .embeddings = embedding_model_,
                // llama2:7b-chat has dimensions of 4096
                .dimmension = 4096
            });
            chat_memory_ = std::make_shared<EphemeralChatMemory>();

            ChatModelPtr chat_model = std::make_shared<OllamaChat>(ollama_configuration);


            PromptTemplatePtr question_prompt_template = OllamaChat::CreateChatPromptTemplateBuilder()
                    ->AddHumanMessage(R"(
Given the following conversation and a follow up question, rephrase the follow up question to be a standalone question, in its original language.
Chat History:
{chat_history}
Follow Up Input: {question}
Standalone question:)")
            ->Build();

            OutputParserPtr<std::string> string_output_parser = std::make_shared<StringOutputParser>();


            ChainOptions question_chain_options = {
                .input_keys = {"question"},
                .output_keys = {"standalone_question"}
            };
            question_chain_ = std::make_shared<LLMChain<std::string>>(
                chat_model,
                question_prompt_template,
                string_output_parser,
                nullptr,
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
                .input_keys  = {"standalone_question", "context"},
                .output_keys = {"answer"}
            };
            answer_chain_ = std::make_shared<LLMChain<std::string>>(
                chat_model,
                answer_prompt_template,
                string_output_parser,
                nullptr,
                answer_chain_options
                );

            RAGChainOptions rag_chain_options = {
                .condense_question_key = question_chain_options.output_keys[0],
                .context_output_key = "context",
            };
            rag_chain_ = std::make_shared<RAGChain<std::string>>(
                chat_memory_,
                retriever_,
                question_chain_,
                answer_chain_
                );
        }

        EmbeddingsPtr embedding_model_;
        RetrieverPtr retriever_;
        ChatMemoryPtr chat_memory_;
        ChainPtr<std::string> question_chain_;
        ChainPtr<std::string> answer_chain_;
        RAGChainPtr<std::string> rag_chain_;
    };

    TEST_F(RAGChainTest, SimpleQAChat) {
        // run with empty docs
        const auto ctx1 = ContextMutataor::Create()
        ->Put("question", "why sea is blue?")
        ->Build();

        auto output = rag_chain_->Invoke(ctx1);
        std::cout << "output = " << output << std::endl;

        // invoke again to verify chat_histroy
        const auto ctx2 = ContextMutataor::Create()
        ->Put("question", "Can you explain in a way that even 6-year child could understand?")
        ->Build();
        output = rag_chain_->Invoke(ctx2);
        std::cout << "output = " << output << std::endl;

        // create a new context and verify
        auto ctx_builder = ContextMutataor::Create();
        chat_memory_->LoadMemories(ctx_builder);
        auto ctx3 = ctx_builder->Build();
        auto memory_key = chat_memory_->GetOutputKeys()[0];
        ASSERT_TRUE(ctx3->values().contains(memory_key));
        ASSERT_TRUE(ctx3->values().at(memory_key).has_string_value());
        std::cout << ctx3->values().at(memory_key).string_value() << std::endl;
        ASSERT_TRUE(ctx3->values().at(memory_key).string_value().find("why sea is blue?") != std::string::npos);

    }
}
