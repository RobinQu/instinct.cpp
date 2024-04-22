//
// Created by RobinQu on 2024/4/22.
//

#ifndef DUCKDBCONNECTIONPOOL_HPP
#define DUCKDBCONNECTIONPOOL_HPP


#include <duckdb/main/connection.hpp>
#include <utility>

#include "../../../store/duckdb/BaseDuckDBStore.hpp"
#include "../BaseConnectionPool.hpp"
#include "../ManagedConnection.hpp"

namespace INSTINCT_RETRIEVAL_NS {

    class DuckDBConnectionPool final: public BaseConnectionPool<duckdb::Connection> {
        DuckDBPtr db_;
    public:
        explicit DuckDBConnectionPool(DuckDBPtr db)
            : db_(std::move(db)) {
        }

        std::shared_ptr<IConnection<duckdb::Connection>> Create() override {
            return std::make_shared<ManagedConnection<duckdb::Connection>>(
                this->shared_from_this(),
                duckdb::Connection(*db_)
            );
        }
    };
}

#endif //DUCKDBCONNECTIONPOOL_HPP
