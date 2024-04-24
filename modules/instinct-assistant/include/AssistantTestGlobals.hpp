//
// Created by RobinQu on 2024/4/9.
//

#ifndef AGENTTESTGLOBALS_HPP
#define AGENTTESTGLOBALS_HPP


#include <gtest/gtest.h>
#include "AssistantGlobals.hpp"
#include "assistant/v2/service/impl/AssistantServiceImpl.hpp"
#include "database/duckdb/DuckDBConnectionPool.hpp"
#include "database/duckdb/DuckDBDataMapper.hpp"

namespace INSTINCT_ASSISTANT_NS {
    using namespace INSTINCT_DATA_NS;
    using namespace v2;

    class BaseAssistantApiTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
        }

        DuckDBPtr duck_db_ = std::make_shared<DuckDB>(nullptr);
        DuckDBConnectionPoolPtr connection_pool_ = CreateDuckDBConnectionPool(duck_db_);
        DataMapperPtr<AssistantObject, std::string> assistant_data_mapper = CreateDuckDBDataMapper<AssistantObject, std::string>(connection_pool_);
    };





}


#endif //AGENTTESTGLOBALS_HPP
