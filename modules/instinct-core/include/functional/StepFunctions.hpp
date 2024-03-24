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
#include "BaseRunnable.hpp"

namespace INSTINCT_CORE_NS {

    class IStepFunction {
    public:
        virtual ~IStepFunction()=default;

        [[nodiscard]] virtual std::vector<std::string> GetInputKeys() const = 0;

        [[nodiscard]] virtual std::vector<std::string> GetOutputKeys() const = 0;

        virtual void ValidateInput(const JSONContextPtr &input) = 0;
    };

    using GenericChain = BaseRunnable<JSONContextPtr, JSONContextPtr>;
    using GenericChainPtr = RunnablePtr<JSONContextPtr, JSONContextPtr>;

    class BaseStepFunction : public virtual IStepFunction, public GenericChain {

    public:
        void ValidateInput(const JSONContextPtr &input) override {
            for (const auto &k: this->GetInputKeys()) {
                assert_true(input->Contains(k), "context should contain key " + k);
            }
        }
    };

    using StepFunctionPtr = std::shared_ptr<BaseStepFunction>;

    class SequenceStepFunction final : public BaseStepFunction {
        std::vector<StepFunctionPtr> steps_;

    public:
        explicit SequenceStepFunction(const std::vector<StepFunctionPtr> &steps) : steps_(steps) {
            erase_if(steps_, [](const auto &step) {
                return step == nullptr;
            });
            assert_true(!steps_.empty(), "Steps cannot be empty");
        }

        [[nodiscard]] const std::vector<StepFunctionPtr> &GetSteps() const {
            return steps_;
        }


        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            JSONContextPtr continuous_input = input;
            for (const auto &step: steps_) {
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


    static const std::string RETURN_VALUE_VARIABLE_KEY = "__return_value__";


    class FunctionReducer {
        StepFunctionPtr step_function_;
    public:
        explicit FunctionReducer(StepFunctionPtr stepFunction) :
                step_function_(std::move(stepFunction)) {}

    public:
        [[nodiscard]] nlohmann::json reduce(const JSONContextPtr &ctx) const {
            auto result = step_function_->Invoke(ctx);
            return result->GetPayload().at(RETURN_VALUE_VARIABLE_KEY);
        }

        nlohmann::json operator()(const JSONContextPtr &ctx) const {
            return reduce(ctx);
        }
    };

    class GetterReducer {
        std::string variable_name_;
    public:
        explicit GetterReducer(std::string variableName) : variable_name_(std::move(variableName)) {}

        [[nodiscard]] nlohmann::json reduce(const JSONContextPtr &ctx) const {
            return ctx->GetPayload().at(variable_name_);
        }

        nlohmann::json operator()(const JSONContextPtr &ctx) const {
            return reduce(ctx);
        }
    };

    using JSONContextReducer = std::function<nlohmann::json(const JSONContextPtr &)>;

    class MappingStepFunction final : public BaseStepFunction {
        std::unordered_map<std::string, JSONContextReducer> steps_{};
    public:
        explicit MappingStepFunction(std::unordered_map<std::string, JSONContextReducer> steps)
                : steps_(std::move(steps)) {
            assert_true(!steps_.empty(), "Steps cannot be empty");
        }

        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            auto output_context = CreateJSONContext();
            for (const auto &[k, v]: steps_) {
                output_context->GetPayload()[k] = v(input);
            }
            return output_context;
        }

        std::vector<std::string> GetInputKeys() const override {
//            std::unordered_set<std::string> keys;
//            for (const auto& v: std::views::values(steps_)) {
//                keys.insert(v->GetInputKeys().begin(), v->GetInputKeys().end());
//            }
//            return {keys.begin(), keys.end()};
            return {};
        }

        std::vector<std::string> GetOutputKeys() const override {
            auto name_views = std::views::keys(steps_);
            return {name_views.begin(), name_views.end()};
        }
    };


    class PickingStepFunction: public BaseStepFunction {

    };

    class ReducerStepFunction final: public BaseStepFunction {
        std::string variable_name_;

    public:
        explicit ReducerStepFunction(std::string variable_name)
            : variable_name_(std::move(variable_name)) {
        }

        JSONContextPtr Invoke(const std::shared_ptr<IContext<JSONContextPolicy>>& input) override {
            return CreateJSONContext(input->GetPayload().at(variable_name_));
        }

        [[nodiscard]] std::vector<std::string> GetInputKeys() const override;

        [[nodiscard]] std::vector<std::string> GetOutputKeys() const override;
    };



    using StepLambda = std::function<JSONContextPtr(const JSONContextPtr &context)>;

    class LambdaStepFunction final : public BaseStepFunction {
        StepLambda fn_;
        std::vector<std::string> input_keys_;
        std::vector<std::string> output_keys_;
    public:
        explicit LambdaStepFunction(StepLambda fn,
                                    std::vector<std::string> input_keys = {},
                                    std::vector<std::string> output_keys = {}) :
                fn_(std::move(fn)),
                input_keys_(std::move(input_keys)),
                output_keys_(std::move(output_keys)) {}

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

    class EmptyStepFunction : public BaseStepFunction {
    public:
        [[nodiscard]] std::vector<std::string> GetInputKeys() const override {
            return {};
        }

        [[nodiscard]] std::vector<std::string> GetOutputKeys() const override {
            return {};
        }

        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            return input;
        }
    };


}


#endif //INSTINCT_STEPFUNCTIONS_HPP
