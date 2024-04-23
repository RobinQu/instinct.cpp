//
// Created by RobinQu on 2024/2/28.
//

#ifndef RETRIEVALGLOBALS_HPP
#define RETRIEVALGLOBALS_HPP

#include <retrieval.pb.h>
#include <duckdb.hpp>
#include <inja/environment.hpp>

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

    using DuckDBPtr = std::shared_ptr<duckdb::DuckDB>;


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

        static std::shared_ptr<inja::Environment> create_shared_sql_template_env() {
            auto env = std::make_shared<inja::Environment>();
            env->add_callback("is_non_blank", 1, [](const inja::Arguments& args) {
                auto v = args.at(0)->get<std::string>();
                return StringUtils::IsNotBlankString(v);
            });
            env->add_callback("is_blank", 1, [](const inja::Arguments& args) {
                auto v = args.at(0)->get<std::string>();
                return StringUtils::IsBlankString(v);
            });
            return env;

        }
    }

    static std::shared_ptr<inja::Environment> DEFAULT_SQL_TEMPLATE_INJA_ENV = details::create_shared_sql_template_env();



}


#endif //RETRIEVALGLOBALS_HPP
