//
// Created by RobinQu on 2024/4/22.
//

#ifndef DUCKDBCONNECTIONPOOL_HPP
#define DUCKDBCONNECTIONPOOL_HPP


#include <duckdb.hpp>

#include <instinct/database/duckdb/duckdb_connection.hpp>
#include <instinct/database/base_connection_pool.hpp>
#include <instinct/database/managed_connection.hpp>

namespace INSTINCT_DATA_NS {


    class DuckDBConnectionPool final: public BaseConnectionPool<duckdb::Connection, duckdb::unique_ptr<duckdb::MaterializedQueryResult>> {
        DuckDBPtr db_;
    public:

        explicit DuckDBConnectionPool(DuckDBPtr db, const ConnectionPoolOptions &options)
            : BaseConnectionPool(options), db_(std::move(db)) {
        }

        std::shared_ptr<IConnection<duckdb::Connection, duckdb::unique_ptr<duckdb::MaterializedQueryResult>>> Create() override {
            return std::make_shared<DuckDBConnection>(
                std::make_unique<duckdb::Connection>(*db_)
            );
        }

        /**
         * As DuckDB is a local embedded database and has none connection turbulence, we canno always return true.
         * @param connection
         * @return
         */
        bool Check(const IConnectionPool::ConnectionPtr &connection) override {
            return true;
        }
    };

    using DuckDBConnectionPoolPtr = ConnectionPoolPtr<duckdb::Connection, duckdb::unique_ptr<duckdb::MaterializedQueryResult>>;

    static DuckDBConnectionPoolPtr CreateDuckDBConnectionPool(const DuckDBPtr& db, const ConnectionPoolOptions &options = {}) {
        const auto pool =  std::make_shared<DuckDBConnectionPool>(db, options);
        pool->Initialize();
        return pool;
    }
}

#endif //DUCKDBCONNECTIONPOOL_HPP
