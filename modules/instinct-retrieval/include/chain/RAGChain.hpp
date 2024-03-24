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
#include "functional/Xn.hpp"
#include "functional/Op.hpp"
#include "prompt/PlainPromptTemplate.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    /**
     *
     * @tparam Input
     * @tparam Output
     * @param retriever to fetch external knowledge (or documents)
     * @param chat_memory conversation memory
     * @param options RAG related options
     * @return
     */
    static GenericChainPtr CreateConversationalRAG(
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
        auto question_fn = xn::steps::mapping({
            {
                "standalone_question", xn::reducers::return_value(xn::steps::mapping({
                    {"question",     xn::reducers::selection("question")},
                    {"chat_history", xn::reducers::return_value(chat_memory->AsLoadMemoryFunction())}
                }) | question_prompt_template | model->AsModelfunction() | xn::operators::stringfy_generation)
            }
        });

        auto context_fn = xn::steps::mapping({
                                                            {"context",             xn::reducers::return_value(
                                                                    retriever->AsContextRetrieverFunction(
                                                                            {.text_query_variable_key = "standalone_question"}))},
                                                            {"standalone_question", xn::reducers::selection(
                                                                    "standalone_question")},
                                                            {"question",            xn::reducers::selection("question")}
                                                    });


        auto answer_fn = xn::steps::mapping({
                                                           {"answer",   xn::reducers::return_value(
                                                                   answer_prompt_template | model->AsModelfunction())},
                                                           {"question", xn::reducers::selection("question")}
                                                   });


        return question_fn
               | context_fn
               | answer_fn
               | chat_memory->AsSaveMemoryFunction(
                {.prompt_variable_key = "question", .answer_variable_key = "answer"});


    }


}


#endif //RAGCHAIN_HPP
