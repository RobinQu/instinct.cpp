//
// Created by RobinQu on 2024/4/23.
//
#include <gtest/gtest.h>
#include "RetrievalGlobals.hpp"
#include "tools/orm/duckdb/DuckDBConnectionPool.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    class DuckDBConnectionPoolTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
        }

        DuckDBPtr mem_db_  = std::make_shared<duckdb::DuckDB>(nullptr);
    };

    TEST_F(DuckDBConnectionPoolTest, TestAcquireRelease) {
        constexpr ConnectionPoolOptions options {.intial_connection_count = 2};
        auto pool = CreateDuckDBConnectionPool(mem_db_, options);
        auto c1 = pool->Acquire();
        auto c2 = pool->Acquire();

        ASSERT_THROW({
            // only two connections in the pool
            pool->Acquire();
        }, InstinctException);

        pool->Release(c1);

        ASSERT_NO_THROW({
            // should normally acquire since one is available
            pool->Acquire();
        });
    }



}

