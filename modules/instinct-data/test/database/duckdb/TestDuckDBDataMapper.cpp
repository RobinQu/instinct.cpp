//
// Created by RobinQu on 2024/4/23.
//
#include <gtest/gtest.h>
#include <llm.pb.h>
#include "database/duckdb/DuckDBConnectionPool.hpp"
#include "database/duckdb/DuckDBDataTemplate.hpp"


namespace INSTINCT_DATA_NS {
    // using namespace INSTINCT_CORE_NS;

    class TestDuckDBDataMapper: public testing::Test {
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
            assert_query_ok(result);
        }

        DuckDBPtr db_ = std::make_shared<duckdb::DuckDB>(nullptr);
        DuckDBConnectionPoolPtr pool_ = CreateDuckDBConnectionPool(db_);
    };

    TEST_F(TestDuckDBDataMapper, SimpleCRUD) {
        auto data_mapper = CreateDuckDBDataMapper<llm::Message, std::string>(pool_);
        auto id1 = data_mapper->InsertOne("insert into message(role, content) values({{text(role)}}, {{text(content)}}) returning (id);", {
            {"role", "human"},
            {"content", "why sky is blue?"}
        });

        auto id2_id3 = data_mapper->InsertMany(R"(
insert into message(role, content) values
## for message in messages
 ({{text(message.role)}}, {{text(message.content)}}),
## endfor
returning (id);
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

        auto count = data_mapper->Execute("select count(*) from message;", {});
        ASSERT_EQ(count, 3);

        const auto msg1= data_mapper->SelectOne("select * from message where id = {{text(id)}};", {{"id", id1}});
        ASSERT_EQ(msg1->role(), "human");
        ASSERT_EQ(msg1->content(), "why sky is blue?");

        const auto msg1_msg3 = data_mapper->SelectMany("select * from message where role = 'human';", {});
        ASSERT_EQ(msg1_msg3.size(), 2);
        for(const auto& msg: msg1_msg3) {
            ASSERT_EQ("human", msg.role());
        }

        count = data_mapper->Execute("update message set content = 'You are super AI, aka SKYNET.' where role ='system';", {});
        ASSERT_EQ(count, 1);
        const auto msg2 = data_mapper->SelectOne("select * from message where role ='system';", {});
        ASSERT_EQ(msg2->content(), "You are super AI, aka SKYNET.");


        count = data_mapper->Execute("delete from message where role = 'human';", {});
        ASSERT_EQ(count, 2);

        count = data_mapper->Execute("select count(*) from message;", {});
        ASSERT_EQ(count, 1);
    }
}

