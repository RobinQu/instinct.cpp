//
// Created by RobinQu on 3/21/24.
//

#ifndef INSTINCT_STEPFUNCTIONS_HPP
#define INSTINCT_STEPFUNCTIONS_HPP
#include <utility>

#include "CoreGlobals.hpp"
#include "IRunnable.hpp"
#include "JSONContextPolicy.hpp"
#include "tools/Assertions.hpp"

namespace INSTINCT_CORE_NS {

    class IStepFunction: public virtual IRunnable<JSONContextPtr , JSONContextPtr> {
    public:
        [[nodiscard]] virtual std::vector<std::string> GetInputKeys() const = 0;
        [[nodiscard]] virtual std::vector<std::string> GetOutputKeys() const = 0;
        virtual void ValidateInput(const JSONContextPtr& input) = 0;
    };

    using StepFunctionPtr = std::shared_ptr<IStepFunction>;

    class BaseStepFunction: public virtual IStepFunction {

    public:
        JSONContextPtr Invoke(const JSONContextPtr &input) override = 0;

        AsyncIterator<JSONContextPtr> Batch(const std::vector<JSONContextPtr> &input) override {
            return rpp::source::from_iterable(input)
                   | rpp::operators::map([&](const auto& ctx) {
                return Invoke(ctx);
            });
        }

        AsyncIterator<JSONContextPtr> Stream(const JSONContextPtr &input) override {
            return rpp::source::from_callable([&]() {
                return Invoke(input);
            });
        }

        void ValidateInput(const JSONContextPtr &input) override {
            for (const auto& k: this->GetInputKeys()) {
                assert_true(input->Contains(k), "context should contain key " + k);
            }
        }
    };

    class SequenceStepFunction final: public BaseStepFunction {
        std::vector<StepFunctionPtr> steps_;

    public:
        explicit SequenceStepFunction(const std::vector<StepFunctionPtr> &steps) : steps_(steps) {
            assert_true(!steps_.empty(), "Steps cannot be empty");
        }

        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            JSONContextPtr continuous_input = input;
            for (const auto& step: steps_) {
                continuous_input = step->Invoke(continuous_input);
            }
            return continuous_input;
        }

        [[nodiscard]] std::vector<std::string> GetInputKeys() const override {
            return steps_.front()->GetInputKeys();
        }

        [[nodiscard]] std::vector<std::string> GetOutputKeys() const override {
            return steps_.front()->GetOutputKeys();
        }


    };

    class MappingStepFunction final: public BaseStepFunction {
        std::unordered_map<std::string, StepFunctionPtr> steps_{};
    public:
        explicit MappingStepFunction(std::unordered_map<std::string, StepFunctionPtr> steps)
                : steps_(std::move(steps)) {
            assert_true(!steps_.empty(), "Steps cannot be empty");
        }

        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            auto output_context = CreateJSONContext();
            for (const auto& [k,v]: steps_) {
                auto result = v->Invoke(input);
                output_context->PutContext(k, *result);
            }
            return output_context;
        }

        std::vector<std::string> GetInputKeys() const override {
            std::unordered_set<std::string> keys;
            for (const auto& v: std::views::values(steps_)) {
                keys.insert(v->GetInputKeys().begin(), v->GetInputKeys().end());
            }
            return {keys.begin(), keys.end()};
        }

        std::vector<std::string> GetOutputKeys() const override {
            auto name_views = std::views::keys(steps_);
            return {name_views.begin(), name_views.end()};
        }
    };


    using StepLambda = std::function<JSONContextPtr (const JSONContextPtr& context)>;
    class LambdaStepFunction final: public BaseStepFunction {
        StepLambda fn_;
        std::vector<std::string> input_keys_;
        std::vector<std::string> output_keys_;
    public:
        explicit LambdaStepFunction(StepLambda fn,
                                    const std::vector<std::string>& input_keys = {},
                                    const std::vector<std::string>& output_keys = {}) :
                            fn_(std::move(fn)),
                            input_keys_(input_keys),
                            output_keys_(output_keys) {}

        [[nodiscard]] std::vector<std::string> GetInputKeys() const override {
            return input_keys_;
        }

        [[nodiscard]] std::vector<std::string> GetOutputKeys() const override {
            return output_keys_;
        }

        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            return std::invoke(fn_, input);
        }
    };

}



#endif //INSTINCT_STEPFUNCTIONS_HPP
