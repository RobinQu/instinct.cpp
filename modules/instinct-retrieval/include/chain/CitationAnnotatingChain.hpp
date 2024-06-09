//
// Created by RobinQu on 2024/6/8.
//

#ifndef FILECITATIONANNOTATIONCHAIN_HPP
#define FILECITATIONANNOTATIONCHAIN_HPP

#include "RetrievalGlobals.hpp"
#include "chain/MessageChain.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    using CitationAnnotatingChain = MessageChain<CitationAnnotatingContext, AnswerWithCitations>;
    using CitationAnnotatingChainPtr = MessageChainPtr<CitationAnnotatingContext, AnswerWithCitations>;

    namespace details {
        static JSONContextPtr parse_citation_annotating_context(const CitationAnnotatingContext& context) {
            assert_true(context.original_search_response().entries_size() > 0, "should provide at least one search entry");
            std::string context_string;
            for(int i=0;const auto& entry: context.original_search_response().entries()) {
                  context_string += std::to_string(++i);
                  context_string += ". ";
                  context_string += entry.content();
            }
            auto mapping_context =  CreateJSONContext({
                {"context", context_string},
                {"answer", context.original_answer()},
                {"question", context.question()}
            });
            mapping_context->RequireMappingData()["docs"] = CreateJSONContext(context.original_search_response());
            return mapping_context;
        }

        static AnswerWithCitations parse_answer_with_citations(const JSONContextPtr& context) {
            static std::regex ANNOTATION_REGEX {R"(\(source\.(\d+)?\))"};
            AnswerWithCitations answer_with_citations;
            const auto final_answer = context->RequireMappingData().at("final_answer")->RequirePrimitive<std::string>();
            answer_with_citations.set_answer(final_answer);
            const auto original_search_response = context->RequireMappingData().at("docs")->RequireMessage<SearchToolResponse>();
            for(const auto& match: StringUtils::MatchPattern(final_answer, ANNOTATION_REGEX)) {
                if (match.size()==2) {
                    if (const auto quotation_index = std::stoi(match[1]); quotation_index < original_search_response.entries_size()) {
                        auto* citation = answer_with_citations.add_citations();
                        citation->set_annotation(match[0].str());
                        citation->mutable_quote()->CopyFrom(original_search_response.entries(quotation_index));
                    }
                    LOG_WARN("invalid quotation index for annoation: {}", match.str());
                } else {
                    LOG_WARN("invalid match for annoation: {}", match.str());
                }
            }
            return answer_with_citations;
        }
    }


    struct CitationAnnotatingChainOptions {
        ChainOptions chain_options;
    };

    static CitationAnnotatingChainPtr CreateCitationAnnotatingChain(
        const ChatModelPtr& chat_model,
        PromptTemplatePtr prompt_template = nullptr,
        const CitationAnnotatingChainOptions options = {}
    ) {
        if (!prompt_template) {
            prompt_template = CreatePlainPromptTemplate(R"(You are a serious researcher. Please try your best to rewrite the answer with citation to the context information. For every sentence you write, cite the book name and paragraph number as (source.1) or (source.1),(source.2), ...,(source.n) for multiple source references.
Here is an example.

Context information:
1. The attendees became the leaders of AI research in the 1960s.
2. They and their students produced programs that the press described as 'astonishing'
3. computers were learning checkers strategies, solving word problems in algebra,
4. proving logical theorems and speaking English.  By the middle of the 1960s, research in
5. the U.S. was heavily funded by the Department of Defense and laboratories had been
6. established around the world. Herbert Simon predicted, 'machines will be capable,
7. within twenty years, of doing any work a man can do'.  Marvin Minsky agreed, writing,
8. 'within a generation ... the problem of creating 'artificial intelligence' will
9. substantially be solved'. They had, however, underestimated the difficulty of the problem.
10. Both the U.S. and British governments cut off exploratory research in response
11. to the criticism of Sir James Lighthill and ongoing pressure from the US Congress "
12. to fund more productive projects. Minsky's and Papert's book Perceptrons was understood
13. as proving that artificial neural networks approach would never be useful for solving
14. real-world tasks, thus discrediting the approach altogether.  The 'AI winter', a period
15. when obtaining funding for AI projects was difficult, followed.  In the early 1980s,
16. AI research was revived by the commercial success of expert systems, a form of AI
17. program that simulated the knowledge and analytical skills of human experts. By 1985,
18. the market for AI had reached over a billion dollars. At the same time, Japan's fifth
19. generation computer project inspired the U.S. and British governments to restore funding
20. for academic research. However, beginning with the collapse of the Lisp Machine market in 1987, AI once again fell into disrepute, and a second, longer-lasting winter began.

Question: When did the LISP machine market collapse?
Answer: 1987
Rewrite answer: 1987(source.20)

Let's begin!

Context information:
{context}

Question: {question}
Answer: {answer}
Rewrite answer:)");
        }
        return CreateFunctionalChain(
        CreateLambdaInputParser<CitationAnnotatingContext>(details::parse_citation_annotating_context),
                  CreateLambdaOutputParser<AnswerWithCitations>(details::parse_answer_with_citations),
                  xn::steps::mapping({
                    {"final_answer", prompt_template | chat_model->AsModelFunction()},
                    {"docs", xn::steps::selection("docs")}
                  }),
                  options.chain_options
        );
    }




}


#endif //FILECITATIONANNOTATIONCHAIN_HPP
