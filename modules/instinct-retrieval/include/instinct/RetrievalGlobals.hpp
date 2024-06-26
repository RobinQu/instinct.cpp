//
// Created by RobinQu on 2024/2/28.
//

#ifndef RETRIEVALGLOBALS_HPP
#define RETRIEVALGLOBALS_HPP

#include <retrieval.pb.h>
#include <duckdb.hpp>
#include <instinct/core_global.hpp>
#include <instinct/LLMGlobals.hpp>
#include <instinct/DataGlobals.hpp>

#define INSTINCT_RETRIEVAL_NS instinct::retrieval

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    struct ContextOptions {
        std::string input_variable_key = DEFAULT_PROMPT_VARIABLE_KEY;
        std::string output_variable_key = DEFAULT_ANSWER_OUTPUT_KEY;
    };

    struct RAGChainOptions {
        ChainOptions base_options = {};
    };

}


#endif //RETRIEVALGLOBALS_HPP
