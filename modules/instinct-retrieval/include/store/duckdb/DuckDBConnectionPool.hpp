//
// Created by RobinQu on 2024/4/22.
//

#ifndef DUCKDBCONNECTIONPOOL_HPP
#define DUCKDBCONNECTIONPOOL_HPP


#include <duckdb/main/connection.hpp>

#include "BaseDuckDBStore.hpp"
#include "../../tools/orm/BaseConnectionPool.hpp"
#include "tools/orm/BaseConnection.hpp"

namespace INSTINCT_RETRIEVAL_NS {

    class DuckDBConnection final: public BaseConnection<duckdb::Connection> {
        Connection connection_;
        std::chrono::time_point<std::chrono::system_clock> last_active_time_point_;
    public:
        explicit DuckDBConnection(DuckDB& duck_db, const std::weak_ptr<IConnectionPool<duckdb::Connection>>& connection_pool): BaseConnection<duckdb::Connection>(connection_pool), connection_(duck_db) {}

        Connection * operator*() override {
            return &connection_;
        }

        bool IsAlive() override {
            // connection_.Query("select 1");
            return true;
        }

    };

    class DuckDBConnectionPool final: public BaseConnectionPool<duckdb::Connection> {
        DuckDBPtr db_;
    public:
        std::shared_ptr<IConnection<duckdb::Connection>> Create() override {
            return std::make_shared<DuckDBConnection>(*db_, this->shared_from_this());
        }
    };
}

#endif //DUCKDBCONNECTIONPOOL_HPP
