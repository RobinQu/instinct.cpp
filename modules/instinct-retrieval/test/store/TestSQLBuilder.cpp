//
// Created by RobinQu on 2024/5/31.
//
#include <gtest/gtest.h>

#include <instinct/store/SQLBuilder.hpp>

namespace INSTINCT_RETRIEVAL_NS {
    TEST(TestSQLBuilder, BuildSelectWithTermQuery) {
        SearchQuery search_query;
        ASSERT_EQ(
            SQLBuilder::ToSelectString("foo", "*", search_query),
            "SELECT * FROM foo;"
        );
        search_query.mutable_term()->set_name("bar");
        search_query.mutable_term()->mutable_term()->set_string_value("cow");
        ASSERT_EQ(
            SQLBuilder::ToSelectString("foo", "*", search_query),
            "SELECT * FROM foo WHERE bar = 'cow';"
        );

        Sorter sorter;
        sorter.mutable_field()->set_field_name("kar");
        sorter.mutable_field()->set_order(ASC);
        ASSERT_EQ(
            SQLBuilder::ToSelectString("foo", "*", search_query, {sorter}),
            "SELECT * FROM foo WHERE bar = 'cow' ORDER BY kar ASC;"
        );
        ASSERT_EQ(
            SQLBuilder::ToSelectString("foo", "*", search_query, {sorter}, 10, 20),
            "SELECT * FROM foo WHERE bar = 'cow' ORDER BY kar ASC LIMIT 20 OFFSET 10;"
        );
    }

    TEST(TestSQLBuilder, BuildSelectWithTermsQuery) {
        SearchQuery search_query;
        search_query.mutable_terms()->set_name("bar");
        search_query.mutable_terms()->add_terms()->set_string_value("cow");
        search_query.mutable_terms()->add_terms()->set_string_value("kar");
        ASSERT_EQ(
            SQLBuilder::ToSelectString("foo", "*", search_query),
            "SELECT * FROM foo WHERE bar IN ('cow', 'kar');"
        );
    }

    TEST(TestSQLBuilder, BuildRangeQuery) {
        SearchQuery search_query;
        search_query.mutable_int_range()->set_name("score");
        search_query.mutable_int_range()->set_from(1);
        search_query.mutable_int_range()->set_inclusive_start(true);
        ASSERT_EQ(
                SQLBuilder::ToSelectString("foo", "*", search_query),
                "SELECT * FROM foo WHERE score >= 1;"
            );
        search_query.mutable_int_range()->set_to(100);
        ASSERT_EQ(
                SQLBuilder::ToSelectString("foo", "*", search_query),
                "SELECT * FROM foo WHERE (score >= 1 AND score < 100);"
            );
    }

    TEST(TestSQLBuilder, BuildSelectWithBoolQuery) {
        {
            SearchQuery search_query;
            auto* condition1 = search_query.mutable_bool_()->add_must();
            condition1->mutable_terms()->set_name("bar");
            condition1->mutable_terms()->add_terms()->set_string_value("cow");
            condition1->mutable_terms()->add_terms()->set_string_value("kar");
            auto* condition2 = search_query.mutable_bool_()->add_must();
            condition2->mutable_term()->set_name("bar");
            condition2->mutable_term()->mutable_term()->set_string_value("cow");
            auto* condition3 = search_query.mutable_bool_()->add_should();
            condition3->mutable_int_range()->set_name("score1");
            condition3->mutable_int_range()->set_from(0);
            auto* condition4 = search_query.mutable_bool_()->add_mustnot();
            condition4->mutable_double_range()->set_name("score2");
            condition4->mutable_double_range()->set_to(1.4);
            ASSERT_EQ(
                SQLBuilder::ToSelectString("foo", "*", search_query),
                "SELECT * FROM foo WHERE (bar IN ('cow', 'kar') AND bar = 'cow') AND score1 > 0 AND NOT score2 < 1.4;"
            );
        }

        {
            SearchQuery search_query;
            auto* condition1 = search_query.mutable_bool_()->add_should();
            condition1->mutable_terms()->set_name("bar");
            condition1->mutable_terms()->add_terms()->set_string_value("cow");
            condition1->mutable_terms()->add_terms()->set_string_value("kar");
            auto* condition2 = search_query.mutable_bool_()->add_should();
            condition2->mutable_term()->set_name("bar");
            condition2->mutable_term()->mutable_term()->set_string_value("cow");
            auto* condition3 = search_query.mutable_bool_()->add_mustnot();
            condition3->mutable_int_range()->set_name("score1");
            condition3->mutable_int_range()->set_from(0);
            auto* condition4 = search_query.mutable_bool_()->add_mustnot();
            condition4->mutable_double_range()->set_name("score2");
            condition4->mutable_double_range()->set_to(1.4);
            ASSERT_EQ(
                SQLBuilder::ToSelectString("foo", "*", search_query),
                "SELECT * FROM foo WHERE (bar IN ('cow', 'kar') OR bar = 'cow') AND (NOT score1 > 0 AND NOT score2 < 1.4);"
            );
        }


    }


}
