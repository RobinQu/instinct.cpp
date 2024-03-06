//
// Created by RobinQu on 2024/3/6.
//
#include <gtest/gtest.h>

#include "RetrievalGlobals.hpp"
#include "store/duckdb/DuckDBVectorStore.hpp"

namespace INSTINCT_RETRIEVAL_NS {

    TEST(DuckDBVectorStore, make_create_table_sql) {
        MetadataSchema s1;
        auto* name_field = s1.add_fields();
        name_field->set_name("name");
        name_field->set_type(VARCHAR);
        auto* address_field = s1.add_fields();
        address_field->set_name("adderss");
        address_field->set_type(VARCHAR);
        auto* age_field = s1.add_fields();
        age_field->set_name("age");
        age_field->set_type(INT32);

        const auto sql = details::make_create_table_sql("test_tb1", 128, s1);
        std::cout << sql << std::endl;
        ASSERT_EQ(sql, "CREATE OR REPLACE TABLE test_tb1(id VARCHAR PRIMARY KEY, vector FLOAT[128] NOT NULL, name VARCHAR, adderss VARCHAR, age INTEGER);");
    }


}