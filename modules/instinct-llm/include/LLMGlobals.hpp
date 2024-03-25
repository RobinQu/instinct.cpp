//
// Created by RobinQu on 2024/2/13.
//

#ifndef MODELGLOBALS_H
#define MODELGLOBALS_H

#include "CoreGlobals.hpp"
#include <llm.pb.h>
#include <model/IEmbeddingModel.hpp>

#define INSTINCT_LLM_NS instinct::llm

namespace INSTINCT_LLM_NS {


    using TokenSize = unsigned long;
    using TokenId = unsigned long;

    struct ChainOptions {

    };


    /**
     * this should refer to a Generation
     */
    static const std::string DEFAULT_ANSWER_OUTPUT_KEY = "answer";


    /**
     * this should refer to a PromptValue
     */
    static const std::string DEFAULT_PROMPT_VARIABLE_KEY = "prompt";


    /**
     * this should refer to a string describing a question for retriever
     */
    static const std::string DEFAULT_QUESTION_INPUT_OUTPUT_KEY = "question";

    /**
     * this should refer to a string containing contextual information returned by retriever
     */
    static const std::string DEFAULT_CONTEXT_OUTPUT_KEY = "context";


    static const std::string DEFAULT_STANDALONE_QUESTION_INPUT_KEY = "standalone_question";


    using TemplateVariables = nlohmann::json;
    using TemplateVariablesPtr = std::shared_ptr<TemplateVariables>;

    using PromptExample = nlohmann::json;
    using PromptExamples = std::vector<PromptExample>;

    static TemplateVariablesPtr CreateTemplateVariable() {
        return std::make_shared<TemplateVariables>(nlohmann::json::parse("{}"));
    }



    inline std::ostream& operator<<(std::ostream& ostrm, const Message& msg) {
        ostrm << "Message[role=" << msg.role() << ", content=" << msg.content() << "]";
        return ostrm;
    }

    inline std::ostream& operator<<(std::ostream& ostrm, const core::Embedding& embedding) {
        ostrm << "Embedding[";
        for (const auto& f: embedding) {
            ostrm << f << ", ";
        }
        return ostrm << "]";
    }


    struct ModelOptions {
//        std::string input_prompt_variable_key = DEFAULT_QUESTION_INPUT_OUTPUT_KEY;
//        std::string output_answer_variable_key = DEFAULT_ANSWER_OUTPUT_KEY;
    };



}


#endif //MODELGLOBALS_H
