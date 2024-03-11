//
// Created by RobinQu on 2024/3/11.
//

#ifndef CONTEXTMUTATOR_HPP
#define CONTEXTMUTATOR_HPP

#include <llm.pb.h>
#include <retrieval.pb.h>

#include <utility>

#include "LLMGlobals.hpp"
#include "CoreTypes.hpp"

namespace INSTINCT_LLM_NS {
    // using namespace INSTINCT_LLM_NS;
    using namespace INSTINCT_CORE_NS;
    using namespace google::protobuf;

    class ContextMutataor;
    using ContextMutataorPtr = std::shared_ptr<ContextMutataor>;

    using ContextMutataorValueVariant = std::variant<int32_t, int64_t, float, double, bool, std::string>;

    class ContextMutataor {
        ContextPtr context_;
        std::unordered_map<std::string,ContextMutataorValueVariant> cache_;
    public:
        static ContextMutataorPtr Create() {
            const auto ctx = std::make_shared<LLMChainContext>();
            return Create(ctx);
        }

        static ContextMutataorPtr Create(const ContextPtr& context) {
            return std::make_shared<ContextMutataor>(context);
        }

        static ContextMutataorPtr CreateCopyOf(const ContextPtr& context) {
            const auto ctx = std::make_shared<LLMChainContext>();
            ctx->CopyFrom(*context);
            return Create(ctx);
        }

        explicit ContextMutataor(ContextPtr context)
            : context_(std::move(context)) {
        }

        ContextMutataor* Put(const std::string& name, const ContextMutataorValueVariant& v) {
            cache_[name] = v;
            return this;
        }


        ContextPtr Build() {
            Commit();
            return context_;
        }


        //
        // template<typename T>
        // T Get(const std::string& name, T&& default_value) const {
        //     if (cache_.contains(name)) {
        //         return std::get<T>(cache_.at(name));
        //     }
        //     if (context_.values().contains(name)) {
        //         context_.values().at(name)
        //
        //     }
        //     return default_value;
        // }

        void Rollback() {
            cache_.clear();
        }

        void Commit() {
            for(const auto& [k,value]: cache_) {
                std::visit(overloaded {
                [&](const int32_t v) {
                        PrimitiveVariable pv;
                        pv.set_name(k);
                        pv.set_int_value(v);
                        context_->mutable_values()->emplace(k, pv);
                    },
                    [&](const int64_t v) {
                        PrimitiveVariable pv;
                        pv.set_name(k);
                        pv.set_long_value(v);
                        context_->mutable_values()->emplace(k, pv);
                    },
                    [&](const float v) {
                        PrimitiveVariable pv;
                        pv.set_name(k);
                        pv.set_float_value(v);
                        context_->mutable_values()->emplace(k, pv);
                    },
                    [&](const double v) {
                        PrimitiveVariable pv;
                        pv.set_name(k);
                        pv.set_double_value(v);
                        context_->mutable_values()->emplace(k, pv);
                    },
                    [&](const bool v) {
                        PrimitiveVariable pv;
                        pv.set_name(k);
                        pv.set_bool_value(v);
                        context_->mutable_values()->emplace(k, pv);
                    },
                    [&](const std::string& v) {
                        PrimitiveVariable pv;
                        pv.set_name(k);
                        pv.set_string_value(v);
                        context_->mutable_values()->emplace(k, pv);
                    }
                }, value);
            }

        }
    };


}


#endif //CONTEXTMUTATOR_HPP
