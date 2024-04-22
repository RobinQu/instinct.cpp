//
// Created by RobinQu on 2024/4/22.
//

#ifndef ENTITYDATAMAPPER_HPP
#define ENTITYDATAMAPPER_HPP

#include <store/duckdb/BaseDuckDBStore.hpp>

#include "IDataMapper.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    template<typename T>
    class EntityDataMapper final: public IDataMapper<T> {
        DuckDBPtr db;
    public:
        T SelectOne(const std::string &select_sql) override {

        }

        std::vector<T> SelectMany(const std::string &select_sql) override {

        }

        int Update(const std::string &update_sql) override {

        }

        int Delete(const std::string &delete_sql) override {

        }
    };
}

#endif //ENTITYDATAMAPPER_HPP
