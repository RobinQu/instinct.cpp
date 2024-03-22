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
        [[nodiscard]] virtual std::vector<std::string> GetInputKeys() const = 0;
        [[nodiscard]] virtual std::vector<std::string> GetOutputKeys() const = 0;
        virtual void ValidateInput(const JSONContextPtr& input) = 0;
    };

    class BaseStepFunction: public virtual IStepFunction, public BaseRunnable<JSONContextPtr,JSONContextPtr> {

    public:
        void ValidateInput(const JSONContextPtr &input) override {
            for (const auto& k: this->GetInputKeys()) {
                assert_true(input->Contains(k), "context should contain key " + k);
            }
        }
    };

    using StepFunctionPtr = std::shared_ptr<BaseStepFunction>;

    class SequenceStepFunction final: public BaseStepFunction {
        std::vector<StepFunctionPtr> steps_;

    public:
        explicit SequenceStepFunction(const std::vector<StepFunctionPtr> &steps) : steps_(steps) {
            erase_if(steps_, [](const auto& step) {
               return step == nullptr;
            });
            assert_true(!steps_.empty(), "Steps cannot be empty");
        }

        [[nodiscard]] const std::vector<StepFunctionPtr>& GetSteps() const {
            return steps_;
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

    class EmptyStepFunction: public BaseStepFunction {
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


    StepFunctionPtr operator|(const StepFunctionPtr& first, const StepFunctionPtr& second) {
        std::vector<StepFunctionPtr> steps;
        if (auto first_as_steps = std::dynamic_pointer_cast<SequenceStepFunction>(first)) {
            // try to collapse first as SequenceStepFunction
            steps.insert(steps.end(), first_as_steps->GetSteps().begin(), first_as_steps->GetSteps().end());
        } else if(first != nullptr) {
            steps.push_back(first);
        }

        // try to collapse second as SequenceStepFunction
        if(auto second_as_steps = std::dynamic_pointer_cast<SequenceStepFunction>(second)) {
            steps.insert(steps.end(), second_as_steps->GetSteps().begin(), second_as_steps->GetSteps().end());
        } else if(second != nullptr) {
            steps.push_back(second);
        }
        if(steps.empty()) {
            return std::make_shared<EmptyStepFunction>();
        }
        return std::make_shared<SequenceStepFunction>(steps);
    }

    StepFunctionPtr operator|(const StepFunctionPtr& first, const StepLambda& step_lambda) {
        return first | std::make_shared<LambdaStepFunction>(step_lambda);
    }


}



#endif //INSTINCT_STEPFUNCTIONS_HPP
