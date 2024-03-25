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
#include "prompt/PlainPromptTemplate.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;
    using namespace INSTINCT_CORE_NS;

    /**
     *
     * @tparam Input
     * @tparam Output
     * @param retriever to fetch external knowledge (or documents)
     * @param chat_memory conversation memory
     * @param options RAG related options
     * @return
     */
    static TextChainPtr CreateTextRAGChain(
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
//                    .output_keys = {"question"}
            });
        }

        if (!answer_prompt_template) {
            answer_prompt_template = CreatePlainPromptTemplate(R"(Answer the question based only on the following context:
{context}

Question: {standalone_question}

)", {.input_keys= {"context", "standalone_question"}});
        }
        auto input_parser = std::make_shared<PromptValueVariantInputParser>();
        auto output_parser = std::make_shared<StringOutputParser>();

        auto question_fn = xn::steps::mapping({
                                                      {
                                                              "standalone_question", xn::steps::mapping({
                                                                                                                {"question",     xn::steps::passthrough()},
                                                                                                                {"chat_history", chat_memory->AsLoadMemoryFunction() | xn::steps::combine_chat_history()}
                                                                                                        }) | question_prompt_template | model->AsModelfunction() | xn::steps::stringify_generation()
                                                      },
                                                      {
                                                              "question", xn::steps::passthrough()
                                                      }
                                              });

        auto context_fn = xn::steps::mapping({
                                                     {"context", xn::steps::selection("question") | retriever->AsContextRetrieverFunction()},
                                                     {"standalone_question", xn::steps::selection("standalone_question")},
                                                     {"question",            xn::steps::selection("question")},
                                             });

        auto answer_fn = xn::steps::mapping({
                                                    {"answer",   answer_prompt_template | model->AsModelfunction()},
                                                    {"question", xn::steps::selection("question")}
                                            });

        auto fn = question_fn
               | context_fn
               | answer_fn
               | chat_memory->AsSaveMemoryFunction(
                {.is_question_string = true, .prompt_variable_key = "question", .answer_variable_key = "answer"})
               | xn::steps::selection("answer");


        return CreateFunctionalChain<PromptValueVariant,std::string>(
                input_parser,
                output_parser,
                fn,
                options.base_options
                );
    }


}


#endif //RAGCHAIN_HPP
