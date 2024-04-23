//
// Created by RobinQu on 2024/4/22.
//

#ifndef DUCKDBCONNECTIONPOOL_HPP
#define DUCKDBCONNECTIONPOOL_HPP


#include <duckdb.hpp>

#include "../BaseConnectionPool.hpp"
#include "../ManagedConnection.hpp"

namespace INSTINCT_DATA_NS {


    class DuckDBConnectionPool final: public BaseConnectionPool<duckdb::Connection> {
        DuckDBPtr db_;
    public:
        explicit DuckDBConnectionPool(DuckDBPtr db, const ConnectionPoolOptions &options)
            : BaseConnectionPool<duckdb::Connection>(options), db_(std::move(db)) {
        }

        std::shared_ptr<IConnection<duckdb::Connection>> Create() override {
            return std::make_shared<ManagedConnection<duckdb::Connection>>(
                std::make_unique<duckdb::Connection>(*db_)
            );
        }

        /**
         * As DuckDB is a local embedded database and has none connection turbulence, we canno always return true.
         * @param connection
         * @return
         */
        bool Check(const typename IConnectionPool<duckdb::Connection>::ConnectionPtr &connection) override {
            return true;
        }
    };

    using DuckDBConnectionPoolPtr = ConnectionPoolPtr<duckdb::Connection>;

    static DuckDBConnectionPoolPtr CreateDuckDBConnectionPool(const DuckDBPtr& db, const ConnectionPoolOptions &options = {}) {
        const auto pool =  std::make_shared<DuckDBConnectionPool>(db, options);
        pool->Initialize();
        return pool;
    }
}

#endif //DUCKDBCONNECTIONPOOL_HPP
