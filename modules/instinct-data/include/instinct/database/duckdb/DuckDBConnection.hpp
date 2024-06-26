//
// Created by RobinQu on 2024/4/24.
//

#ifndef DUCKDBCONNECTION_HPP
#define DUCKDBCONNECTION_HPP

#include <instinct/database//ManagedConnection.hpp>

namespace INSTINCT_DATA_NS {
    class DuckDBConnection final: public ManagedConnection<duckdb::Connection, duckdb::unique_ptr<duckdb::MaterializedQueryResult>> {
    public:
        explicit DuckDBConnection(std::unique_ptr<duckdb::Connection> impl, const std::string &id = StringUtils::GenerateUUIDString())
            : ManagedConnection(std::move(impl), id, DEFAULT_SQL_TEMPLATE_INJA_ENV) {
        }

    private:
        duckdb::unique_ptr<duckdb::MaterializedQueryResult> Execute(const std::string &sql_line) override {
            return GetImpl().Query(sql_line);
        }
    };
}

#endif //DUCKDBCONNECTION_HPP
