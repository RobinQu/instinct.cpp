//
// Created by RobinQu on 2024/4/23.
//

#ifndef DATAGLOBALS_HPP
#define DATAGLOBALS_HPP

#ifndef INSTINCT_DATA_NS
#define INSTINCT_DATA_NS instinct::data
#endif

#include <data.pb.h>
#include <duckdb.hpp>
#include <inja/inja.hpp>

#include "CoreGlobals.hpp"
#include "tools/StringUtils.hpp"


namespace INSTINCT_DATA_NS {
    using namespace INSTINCT_CORE_NS;

    using DuckDBPtr = std::shared_ptr<duckdb::DuckDB>;

    using SQLTemplate = std::string_view;

    using SQLContext = nlohmann::ordered_json;


    namespace details {
        using namespace duckdb;

        static std::shared_ptr<inja::Environment> create_shared_sql_template_env() {
            auto env = std::make_shared<inja::Environment>();
            env->add_callback("is_not_blank", 1, [](const inja::Arguments& args) {
                const auto v = args.at(0)->get<std::string>();
                return StringUtils::IsNotBlankString(v);
            });
            env->add_callback("is_blank", 1, [](const inja::Arguments& args) {
                const auto v = args.at(0)->get<std::string>();
                return StringUtils::IsBlankString(v);
            });
            env->add_callback("text", 1, [](const inja::Arguments& args) {
                if (const auto& arg = args.at(0); arg->is_string() && !arg->get<std::string>().empty()) {
                    const auto v = arg->get<std::string>();
                    return "'" + StringUtils::EscapeSQLText(v) + "'";
                }
                return std::string {"NULL"};

            });
            env->add_callback("stringify", 1, [](const inja::Arguments& args) {
                if (const auto& arg = args.at(0); arg->is_array() || arg->is_object()) {
                    const auto v = arg->dump();
                    return "'" + StringUtils::EscapeSQLText(v) + "'";
                }
                return std::string {"NULL"};
            });

            env->add_callback("timestamp", 1, [](const inja::Arguments& args) {
                const auto v = args.at(0)->get<int64_t>();
                // TIMESTAMP in sql database has micro-second precision
                return "'" + Timestamp::ToString(Timestamp::FromEpochMicroSeconds(v)) + "'";
            });

            env->set_trim_blocks(true);
            env->set_lstrip_blocks(true);
            return env;
        }

    }


    static std::string to_string(const OSSStatus_ErrorType& type) {
        if (type == OSSStatus_ErrorType_ObjectNotFound) {
            return "ObjectNotFound";
        }
        // for rest of enum values
        return "UnknwnOSSError";
    }


    static bool check_query_ok(const duckdb::unique_ptr<duckdb::MaterializedQueryResult>& result) {
        return !result->GetErrorObject().HasError();
    }

    static void assert_query_ok(const duckdb::unique_ptr<duckdb::MaterializedQueryResult>& result) {
        if (!check_query_ok(result)) {
            throw ClientException(result->GetError());
        }
    }

    static void assert_query_ok(const duckdb::unique_ptr<duckdb::QueryResult>& result) {
        if (const auto error = result->GetErrorObject(); error.HasError()) {
            throw ClientException(result->GetError());
        }
    }

    static void assert_prepared_ok(const duckdb::unique_ptr<duckdb::PreparedStatement>& result,
                                   const std::string& msg = "Failed to prepare statement") {
        if (const auto error = result->GetErrorObject(); error.HasError()) {
            throw ClientException(msg + " " + result->GetError());
        }
    }

    static void assert_status_ok(const OSSStatus& status) {
        if (status.has_error()) {
            throw ClientException(fmt::format("ObjectStore operation failed. error_type={} ,message={}", to_string(status.error_type()), status.message()));
        }
    }

    static std::shared_ptr<inja::Environment> DEFAULT_SQL_TEMPLATE_INJA_ENV = details::create_shared_sql_template_env();



}

#endif //DATAGLOBALS_HPP
