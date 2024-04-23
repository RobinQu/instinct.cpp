//
// Created by RobinQu on 2024/4/23.
//
#include <gtest/gtest.h>

#include "tools/orm/duckdb/DuckDBConnectionPool.hpp"
#include "tools/orm/duckdb/DuckDBDataMapper.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_CORE_NS;

    class TestDuckdDBDataMapper: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();

            Connection connection {*db_};
            auto result = connection.Query(R"(
CREATE SEQUENCE id_sequence START 1;
create table message (
    id integer DEFAULT nextval('id_sequence'),
    role varchar not null,
    content varchar not null
);
)");
            details::assert_query_ok(result);
        }

        DuckDBPtr db_ = std::make_shared<duckdb::DuckDB>(nullptr);
        DuckDBConnectionPoolPtr pool_ = CreateDuckDBConnectionPool(db_);
    };

    TEST_F(TestDuckdDBDataMapper, SimpleCRUD) {
        DuckDBDataMapper<Message> data_mapper {pool_};
        auto id1 = data_mapper.InsertOne("insert into message(role, content) values({{role}}, {{content}})", {
            {"role", "human"},
            {"content", "why sky is blue?"}
        });

        auto id2_id3 = data_mapper.InsertMany(R"(
insert into message(role, content) values
## for message in messages
 ({{message.role}}, {{message.content}})
## endfor
)", {
            {"messages", {
                {
                  {"role", "system"},
                    {"content", "You are an helpful AI assistant"}
                },
                {
                    {"role", "human"},
                    {"content", "What's 1 plus one?"}
                }
            }}
        });
        ASSERT_EQ(id2_id3.size(), 2);

        auto count = data_mapper.Execute("select count(*) from message;", {});
        ASSERT_EQ(count, 3);

        const auto msg1= data_mapper.SelectOne("select * from message;", {{"id", id1}});
        ASSERT_EQ(msg1->role(), "human");
        ASSERT_EQ(msg1->content(), "why sky is blue?");

        const auto msg1_msg3 = data_mapper.SelectMany("select * from message where role = 'human';", {});
        ASSERT_EQ(msg1_msg3.size(), 2);
        for(const auto& msg: msg1_msg3) {
            ASSERT_EQ("human", msg.role());
        }

        count = data_mapper.Execute("update message set message = 'You are super AI, aka SKYNET.' where role ='system';", {});
        ASSERT_EQ(count, 1);
        const auto msg2 = data_mapper.SelectOne("select * from message where role ='system';", {});
        ASSERT_EQ(msg2->content(), "You are super AI, aka SKYNET.");


        count = data_mapper.Execute("delete from message where role = 'human';", {});
        ASSERT_EQ(count, 2);

        count = data_mapper.Execute("select count(*) from message;", {});
        ASSERT_EQ(count, 1);
    }
}

