//
// Created by RobinQu on 2024/2/28.
//

#ifndef RETRIEVALGLOBALS_HPP
#define RETRIEVALGLOBALS_HPP

#include <retrieval.pb.h>

#include <utility>
#include "CoreGlobals.hpp"
#include "LLMGlobals.hpp"

#define INSTINCT_RETRIEVAL_NS instinct::retrieval

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    struct ContextOptions {
        std::string input_variable_key = DEFAULT_PROMPT_VARIABLE_KEY;
        std::string output_variable_key = DEFAULT_ANSWER_OUTPUT_KEY;
    };


    struct RAGChainOptions {
        ChainOptions base_options = {};
//        std::string context_variable_key = DEFAULT_CONTEXT_OUTPUT_KEY;
//        std::string standalone_question_variable_key = DEFAULT_STANDALONE_QUESTION_INPUT_KEY;
//        int top_k = 10;
    };


}


#endif //RETRIEVALGLOBALS_HPP
