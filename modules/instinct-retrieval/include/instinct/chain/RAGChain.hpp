//
// Created by RobinQu on 2024/3/9.
//

#ifndef RAGCHAIN_HPP
#define RAGCHAIN_HPP

#include <utility>

#include <instinct/RetrievalGlobals.hpp>
#include <instinct/chain/MessageChain.hpp>
#include <instinct/chain/LLMChain.hpp>
#include <instinct/retrieval/BaseRetriever.hpp>
#include <instinct/tools/DocumentUtils.hpp>
#include <instinct/functional/Xn.hpp>
#include <instinct/prompt/PlainPromptTemplate.hpp>

#include <instinct/prompt/BasePromptTemplate.hpp>

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;
    using namespace INSTINCT_CORE_NS;

    static StepFunctionPtr CreateConversationalRAGFunction(
            const RetrieverPtr& retriever,
            const ChatModelPtr& model,
            const ChatMemoryPtr& chat_memory,
            PromptTemplatePtr question_prompt_template = nullptr,
            PromptTemplatePtr answer_prompt_template = nullptr,
            const RAGChainOptions& options = {}
        ) {
        if (!question_prompt_template) {
            question_prompt_template = CreatePlainPromptTemplate(R"(
Given the following conversation and a follow up question, rephrase the follow up question to be a standalone question, in its original language.
Chat History:
{chat_history}
Follow Up Input: {question}
Standalone question:)",
             {
                 .input_keys = {"chat_history", "question"},
             });
        }

        if (!answer_prompt_template) {
            answer_prompt_template = CreatePlainPromptTemplate(
                R"(Answer the question based only on the following context:
{context}

Question: {standalone_question}

)", {.input_keys = {"context", "standalone_question"}});
        }

        const auto question_fn = xn::steps::mapping({
            {
                "standalone_question", xn::steps::mapping({
                                           {"question", xn::steps::passthrough()},
                                           {
                                               "chat_history",
                                               chat_memory->AsLoadMemoryFunction() | xn::steps::combine_chat_history()
                                           }
                                       }) | question_prompt_template | model->AsModelFunction() |
                                       xn::steps::stringify_generation()
            },
            {
                "question", xn::steps::passthrough()
            }
        });

        const auto context_fn = xn::steps::mapping({
            {"context", xn::steps::selection("question") | retriever->AsContextRetrieverFunction()},
            {"standalone_question", xn::steps::selection("standalone_question")},
            {"question", xn::steps::selection("question")},
        });

        const auto answer_fn = xn::steps::mapping({
            {"answer", answer_prompt_template | model->AsModelFunction()},
            {"question", xn::steps::selection("question")}
        });

        return  question_fn
                  | context_fn
                  | answer_fn
                  | chat_memory->AsSaveMemoryFunction(
                      {.is_question_string = true, .prompt_variable_key = "question", .answer_variable_key = "answer"})
                  | xn::steps::selection("answer");
    }



    static TextChainPtr CreateTextRAGChain(
        const RetrieverPtr& retriever,
        const ChatModelPtr& model,
        const ChatMemoryPtr& chat_memory,
        const PromptTemplatePtr& question_prompt_template = nullptr,
        const PromptTemplatePtr& answer_prompt_template = nullptr,
        const RAGChainOptions& options = {}
    ) {
        const auto input_parser = std::make_shared<PromptValueVariantInputParser>();
        const auto output_parser = std::make_shared<StringOutputParser>();
        const auto fn = CreateConversationalRAGFunction(retriever, model, chat_memory, question_prompt_template, answer_prompt_template, options);
        return CreateFunctionalChain<PromptValueVariant, std::string>(
            input_parser,
            output_parser,
            fn,
            options.base_options
        );
    }
}


#endif //RAGCHAIN_HPP
