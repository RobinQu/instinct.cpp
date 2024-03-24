//
// Created by RobinQu on 2024/3/9.
//

#ifndef RAGCHAIN_HPP
#define RAGCHAIN_HPP

#include <utility>

#include "RetrievalGlobals.hpp"
#include "chain/MessageChain.hpp"
#include "chain/LLMChain.hpp"
#include "retrieval/BaseRetriever.hpp"
#include "retrieval/DocumentUtils.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    /**
     *
     * @tparam Input
     * @tparam Output
     * @param retriever to fetch external knowledge (or documents)
     * @param question_chain to augmented question prompt. this chain doesn't need memory
     * @param answer_chain to answer final question with context. this chain doesn't need memory
     * @param chat_memory conversation memory
     * @param options RAG related options
     * @return
     */
    static StepFunctionPtr CreateConversationalRAG(
            const RetrieverPtr &retriever,
            const ChatModelPtr &model,
            const ChatMemoryPtr &chat_memory,
            PromptTemplatePtr question_prompt_template = nullptr,
            PromptTemplatePtr answer_prompt_template = nullptr,
            const RAGChainOptions &options = {}
    ) {
        if (!question_prompt_template) {
            question_prompt_template = CreatePlainPromptTemplate(R"(
Given the following conversation and a follow up question, rephrase the follow up question to be a standalone question, in its original language.
Chat History:
{chat_history}
Follow Up Input: {question}
Standalone question:)", {
                    .input_keys = {"chat_history", "question"},
                    .output_keys = {"question"}
            });
        }

        if (!answer_prompt_template) {
            answer_prompt_template = CreatePlainPromptTemplate(R"(Answer the question based only on the following context:
{context}

Question: {standalone_question}

{format_instruction}
)", {.input_keys= {"context", "standalone_question", "format_instruction"}});
        }

        auto stringify_generation = std::make_shared<GenerationToStringFunction>();
        auto question_fn = CreateMappingStepFunction({
            {
                "standalone_question", FunctionReducer(CreateMappingStepFunction({
                    {"question",     GetterReducer("question")},
                    {"chat_history", FunctionReducer(chat_memory->AsLoadMemoryFunction())}
                }) | question_prompt_template | model->AsModelfunction() | stringify_generation)
            }
        });

        auto context_fn = CreateMappingStepFunction({
                                                            {"context",             FunctionReducer(
                                                                    retriever->AsContextRetrieverFunction(
                                                                            {.text_query_variable_key = "standalone_question"}))},
                                                            {"standalone_question", GetterReducer(
                                                                    "standalone_question")},
                                                            {"question",            GetterReducer("question")}
                                                    });


        auto answer_fn = CreateMappingStepFunction({
                                                           {"answer",   FunctionReducer(
                                                                   answer_prompt_template | model->AsModelfunction())},
                                                           {"question", GetterReducer("question")}
                                                   });


        return question_fn
               | context_fn
               | answer_fn
               | chat_memory->AsSaveMemoryFunction(
                {.prompt_variable_key = "question", .answer_variable_key = "answer"});


    }


}


#endif //RAGCHAIN_HPP
