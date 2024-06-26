//
// Created by RobinQu on 3/25/24.
//
#include <gtest/gtest.h>
#include <unordered_map>
#include <core.pb.h>
#include <instinct/functional/Xn.hpp>

namespace INSTINCT_CORE_NS  {

    class XnChainingTest : public ::testing::Test {

    protected:
        void SetUp() override {
            SetupLogging();
            fn1 = xn::steps::lambda([](const JSONContextPtr& ctx) {
                ctx->ProducePrimitive(1);
                return ctx;
            });

            fn2 = xn::steps::lambda([](const JSONContextPtr& ctx) {
                int i = ctx->RequirePrimitive<int>();
                ctx->ProducePrimitive(i * i);
                return ctx;
            });

            fn3 = xn::steps::lambda([](const JSONContextPtr& ctx) {
                int i = ctx->RequirePrimitive<int>();
                ctx->ProducePrimitive(i + i);
                return ctx;
            });

            fn4 = xn::steps::lambda([](const JSONContextPtr& ctx) {
                int i = ctx->RequirePrimitive<int>();
                ctx->ProducePrimitive(i / i);
                return ctx;
            });

            fn5 = xn::steps::lambda([](const JSONContextPtr& ctx) {
                int i = ctx->RequirePrimitive<int>();
                ctx->ProducePrimitive(i - i);
                return ctx;
            });


            fn6 = xn::steps::lambda([](const JSONContextPtr& ctx) {
                Document doc;
                doc.set_text("hello");
                doc.set_id("1111");
                ctx->ProduceMessage(doc);
                return ctx;
            });


            fn7 = xn::steps::lambda([](const JSONContextPtr& ctx) {
                auto doc = ctx->RequireMessage<Document>();
                ctx->ProducePrimitive(doc.text());
                return ctx;
            });

            fn8 = xn::steps::lambda([](const JSONContextPtr& ctx) {
                return ctx;
            });
        }

        StepFunctionPtr fn1;
        StepFunctionPtr fn2;
        StepFunctionPtr fn3;
        StepFunctionPtr fn4;
        StepFunctionPtr fn5;
        StepFunctionPtr fn6;
        StepFunctionPtr fn7;
        StepFunctionPtr fn8;
    };

    TEST_F(XnChainingTest, TestSequenceChaining) {
        auto xn1 = xn::steps::sequence({fn1, fn2, fn3, fn4});
        auto ctx1 = CreateJSONContext();
        auto result1 = xn1->Invoke(ctx1)->RequirePrimitive<int>();

        auto xn2 = fn1 | fn2 | fn3 | fn4;
        auto ctx2 = CreateJSONContext();
        auto result2 = xn2->Invoke(ctx2)->RequirePrimitive<int>();

        ASSERT_EQ(result1, result2);
    }

    TEST_F(XnChainingTest, TestMappingChaining) {
        // using explict construction
        auto xn1 = xn::steps::mapping({
            {"fn2", fn2 },
            {"fn3", fn3},
            {"fn4", fn4}
        });
        auto xn2 = xn::steps::sequence({fn1, xn1});
        auto result1 = xn2->Invoke(CreateJSONContext());
        ASSERT_EQ(result1->RequireMappingData().at("fn2")->RequirePrimitive<int>(), 1);
        ASSERT_EQ(result1->RequireMappingData().at("fn3")->RequirePrimitive<int>(), 2);
        ASSERT_EQ(result1->RequireMappingData().at("fn4")->RequirePrimitive<int>(), 1);

        // using | operator on right-hand side
        auto xn3 = fn1 | xn::steps::mapping( {
                {"fn2", fn2 },
                {"fn3", fn3},
                {"fn4", fn4}
        });
        auto result2 = xn3->Invoke(CreateJSONContext());

        ASSERT_EQ(result1->GetValue().dump(), result2->GetValue().dump());


        // using | operator on left-hand side
        auto xn4 = xn::steps::mapping( {
                {"fn2", fn2 },
                {"fn3", fn3},
                {"fn4", fn4}
        }) | xn::steps::lambda([](const JSONContextPtr& ctx) {
            auto mapping_data = ctx->RequireMappingData();
            int i = mapping_data.at("fn2")->RequirePrimitive<int>() // 4
                        + mapping_data.at("fn3")->RequirePrimitive<int>() //  4
                            + mapping_data.at("fn4")->RequirePrimitive<int>(); // 1
            ctx->ProducePrimitive(i);
            return ctx;
        });
        auto ctx3 = CreateJSONContext();
        ctx3->ProducePrimitive(2);
        auto result3 = xn4->Invoke(ctx3);
        ASSERT_EQ(result3->RequirePrimitive<int>(), 9);
    }

    TEST_F(XnChainingTest, TestWithMessages) {
        auto xn1 = fn6 | fn7;
        auto result1 = xn1->Invoke(CreateJSONContext());
        ASSERT_EQ(result1->RequirePrimitive<std::string>(), "hello");

    }
}
