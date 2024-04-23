//
// Created by RobinQu on 2024/4/23.
//

#ifndef DATAGLOBALS_HPP
#define DATAGLOBALS_HPP

#ifndef INSTINCT_DATA_NS
#define INSTINCT_DATA_NS instinct::data
#endif

#include <duckdb.hpp>
#include <inja/inja.hpp>

#include "CoreGlobals.hpp"
#include "tools/StringUtils.hpp"


namespace INSTINCT_DATA_NS {
    using namespace INSTINCT_CORE_NS;

    using DuckDBPtr = std::shared_ptr<duckdb::DuckDB>;

    namespace details {
        using namespace duckdb;

        static std::shared_ptr<inja::Environment> create_shared_sql_template_env() {
            auto env = std::make_shared<inja::Environment>();
            env->add_callback("is_non_blank", 1, [](const inja::Arguments& args) {
                const auto v = args.at(0)->get<std::string>();
                return StringUtils::IsNotBlankString(v);
            });
            env->add_callback("is_blank", 1, [](const inja::Arguments& args) {
                const auto v = args.at(0)->get<std::string>();
                return StringUtils::IsBlankString(v);
            });

            env->add_callback("text", 1, [](const inja::Arguments& args) {
                auto v = args.at(0)->get<std::string>();
                return "'" + StringUtils::EscapeSQLText(v) + "'";
            });
            env->set_trim_blocks(true);
            env->set_lstrip_blocks(true);
            return env;
        }
    }



    static bool check_query_ok(const duckdb::unique_ptr<duckdb::MaterializedQueryResult>& result) {
        return !result->GetErrorObject().HasError();
    }

    static void assert_query_ok(const duckdb::unique_ptr<duckdb::MaterializedQueryResult>& result) {
        if (!check_query_ok(result)) {
            throw InstinctException(result->GetError());
        }
    }

    static void assert_query_ok(const duckdb::unique_ptr<duckdb::QueryResult>& result) {
        if (const auto error = result->GetErrorObject(); error.HasError()) {
            throw InstinctException(result->GetError());
        }
    }

    static void assert_prepared_ok(const duckdb::unique_ptr<duckdb::PreparedStatement>& result,
                                   const std::string& msg = "Failed to prepare statement") {
        if (const auto error = result->GetErrorObject(); error.HasError()) {
            throw InstinctException(msg + " " + result->GetError());
        }
    }


    static std::shared_ptr<inja::Environment> DEFAULT_SQL_TEMPLATE_INJA_ENV = details::create_shared_sql_template_env();



}

#endif //DATAGLOBALS_HPP
