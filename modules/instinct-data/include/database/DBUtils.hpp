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
        template<typename Impl>
        void RunSQL(const std::filesystem::path& sql_path, const ConnectionPoolPtr<Impl> connection_pool) {
            const auto sql_line = IOUtils::ReadString(sql_path);
            const auto conn = connection_pool->Acquire();
            try {
                
            } catch (...) {}
            connection_pool->Release(conn);
        }

    };

}

#endif //DBUTILS_HPP
