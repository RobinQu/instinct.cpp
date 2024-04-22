//
// Created by RobinQu on 2024/2/28.
//

#ifndef RETRIEVALGLOBALS_HPP
#define RETRIEVALGLOBALS_HPP

#include <retrieval.pb.h>

#include <utility>
#include "CoreGlobals.hpp"
#include "LLMGlobals.hpp"
#include "store/duckdb/DuckDBDocStore.hpp"

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


    namespace details {
        using namespace duckdb;

        static bool check_query_ok(const unique_ptr<MaterializedQueryResult>& result) {
            return !result->GetErrorObject().HasError();
        }

        static void assert_query_ok(const unique_ptr<MaterializedQueryResult>& result) {
            if (!check_query_ok(result)) {
                throw InstinctException(result->GetError());
            }
        }

        static void assert_query_ok(const unique_ptr<QueryResult>& result) {
            if (const auto error = result->GetErrorObject(); error.HasError()) {
                throw InstinctException(result->GetError());
            }
        }

        static void assert_prepared_ok(const unique_ptr<PreparedStatement>& result,
                                       const std::string& msg = "Failed to prepare statement") {
            if (const auto error = result->GetErrorObject(); error.HasError()) {
                throw InstinctException(msg + " " + result->GetError());
            }
        }
    }


}


#endif //RETRIEVALGLOBALS_HPP
