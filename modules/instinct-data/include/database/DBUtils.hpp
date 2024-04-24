//
// Created by RobinQu on 2024/4/22.
//

#ifndef DBUTILS_HPP
#define DBUTILS_HPP

#include "BaseConnectionPool.hpp"
#include "DataGlobals.hpp"
#include "tools/IOUtils.hpp"
#include "tools/SnowflakeIDGenerator.hpp"

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
        static void ExecuteSQL(const std::filesystem::path& sql_path, const ConnectionPoolPtr<ConnectionImpl, QueryResultImpl> connection_pool, const SQLContext& context = {}) {
            const auto sql_line = IOUtils::ReadString(sql_path);
            const auto conn = connection_pool->Acquire();
            try {
                conn->Query(sql_line, context);
            } catch (...) {}
            connection_pool->Release(conn);
        }
    };

}

#endif //DBUTILS_HPP
