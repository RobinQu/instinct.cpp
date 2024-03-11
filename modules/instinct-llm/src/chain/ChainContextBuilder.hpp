//
// Created by RobinQu on 2024/3/10.
//

#ifndef CHAINCONTEXTBUILDER_HPP
#define CHAINCONTEXTBUILDER_HPP


#include <core.pb.h>
#include <llm.pb.h>

#include "CoreTypes.hpp"
#include "LLMGlobals.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    namespace details {
        using ContextValueVariant = std::variant<int32_t, int64_t, float, double, bool, std::string>;
        using ContextValueVariantMap = std::unordered_map<std::string, ContextValueVariant>;
    }

    class ChainContextBuilder {
        details::ContextValueVariantMap values;
    public:
        static std::shared_ptr<ChainContextBuilder> Create() {
            return std::make_shared<ChainContextBuilder>();
        }

        static std::shared_ptr<ChainContextBuilder> Create(const LLMChainContext& ctx) {
            auto builder = Create();
            for (const auto& [k,v]: ctx.values()) {
                builder->Put(k, v);
            }
            return builder;
        }

        ChainContextBuilder* Put(const std::string& name, const details::ContextValueVariant& v) {
            values[name] = v;
            return this;
        }

        ChainContextBuilder* Put(const std::string& name, const PrimitiveVariable& v) {
            switch (v.value_case()) {
                case PrimitiveVariable::kIntValue:
                    values[name] = v.int_value();
                break;
                case PrimitiveVariable::kLongValue:
                    values[name] = v.long_value();
                break;
                case PrimitiveVariable::kFloatValue:
                    values[name] = v.float_value();
                break;
                case PrimitiveVariable::kDoubleValue:
                    values[name] = v.double_value();
                break;
                case PrimitiveVariable::kBoolValue:
                    values[name] = v.bool_value();
                break;
                case PrimitiveVariable::kStringValue:
                    values[name] = v.string_value();
                break;
                default:
                    throw InstinctException("unknown value type for entry: " + name);
            }
            return this;
        }

        void Build(LLMChainContext* ctx) {
            for (const auto& [k,value]: values) {
                std::visit(overloaded {
                [&](const int32_t v) {
                        PrimitiveVariable pv;
                        pv.set_name(k);
                        pv.set_int_value(v);
                        ctx->mutable_values()->emplace(k, pv);
                    },
                    [&](const int64_t v) {
                        PrimitiveVariable pv;
                        pv.set_name(k);
                        pv.set_long_value(v);
                        ctx->mutable_values()->emplace(k, pv);
                    },
                    [&](const float v) {
                        PrimitiveVariable pv;
                        pv.set_name(k);
                        pv.set_float_value(v);
                        ctx->mutable_values()->emplace(k, pv);
                    },
                    [&](const double v) {
                        PrimitiveVariable pv;
                        pv.set_name(k);
                        pv.set_double_value(v);
                        ctx->mutable_values()->emplace(k, pv);
                    },
                    [&](const bool v) {
                        PrimitiveVariable pv;
                        pv.set_name(k);
                        pv.set_bool_value(v);
                        ctx->mutable_values()->emplace(k, pv);
                    },
                    [&](const std::string& v) {
                        PrimitiveVariable pv;
                        pv.set_name(k);
                        pv.set_string_value(v);
                        ctx->mutable_values()->emplace(k, pv);
                    }
                }, value);
            }
        }

        LLMChainContext Build() {
            LLMChainContext ctx;
            Build(&ctx);
            return ctx;
        }

        LLMChainContext* Build(google::protobuf::Arena* arena) {
            auto* ctx = google::protobuf::Arena::Create<LLMChainContext>(arena);
            this->Build(ctx);
            return ctx;
        }

    };

    using ChainContextBuilderPtr = std::shared_ptr<ChainContextBuilder>;
}

#endif //CHAINCONTEXTBUILDER_HPP
