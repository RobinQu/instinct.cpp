//
// Created by RobinQu on 2024/4/22.
//

#ifndef DBUTILS_HPP
#define DBUTILS_HPP

#include <instinct/database/BaseConnectionPool.hpp>
#include <instinct/DataGlobals.hpp>
#include <instinct/tools/IOUtils.hpp>
#include <instinct/tools/SnowflakeIDGenerator.hpp>

namespace INSTINCT_DATA_NS {

    class DBUtils final {
    public:
        /**
         * Exeucte SQL in given file
         * @tparam ConnectionImpl
         * @tparam QueryResultImpl
         * @param sql_path
         * @param connection_pool
         * @param context
         */
        template<typename ConnectionImpl, typename QueryResultImpl>
        static QueryResultImpl ExecuteSQL(const std::filesystem::path& sql_path, const ConnectionPoolPtr<ConnectionImpl, QueryResultImpl> connection_pool, const SQLContext& context = {}) {
            const auto sql_line = IOUtils::ReadString(sql_path);
            return ExecuteSQL(sql_line, connection_pool, context);
        }

        template<typename ConnectionImpl, typename QueryResultImpl>
        static QueryResultImpl ExecuteSQL(const std::string& sql_line, const ConnectionPoolPtr<ConnectionImpl, QueryResultImpl> connection_pool, const SQLContext& context = {}) {
            const auto conn = connection_pool->Acquire();
            QueryResultImpl result;
            try {
                result = std::move(conn->Query(sql_line, context));
            } catch (...) {}
            connection_pool->Release(conn);
            return result;
        }
    };

}

#endif //DBUTILS_HPP
