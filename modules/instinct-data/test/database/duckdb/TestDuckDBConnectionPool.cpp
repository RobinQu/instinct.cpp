//
// Created by RobinQu on 2024/4/23.
//
#include <gtest/gtest.h>
#include <instinct/DataGlobals.hpp>
#include <instinct/database/duckdb/DuckDBConnectionPool.hpp>

namespace INSTINCT_DATA_NS {
    using namespace INSTINCT_CORE_NS;
    class DuckDBConnectionPoolTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
        }

        DuckDBPtr mem_db_  = std::make_shared<duckdb::DuckDB>(nullptr);
    };

    TEST_F(DuckDBConnectionPoolTest, TestAcquireRelease) {
        auto pool = CreateDuckDBConnectionPool(mem_db_, {.initial_connection_count = 2});
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

    TEST_F(DuckDBConnectionPoolTest, TestConnectionGarud) {
        const auto pool = CreateDuckDBConnectionPool(mem_db_, {.initial_connection_count = 2});
        pool->Acquire();
        {
            const auto c = pool->Acquire();
            DuckDBConnectionPool::GuardConnection guard {pool, c};
            ASSERT_FALSE(pool->TryAcquire());
        }
        ASSERT_TRUE(pool->Acquire());
    }

}

