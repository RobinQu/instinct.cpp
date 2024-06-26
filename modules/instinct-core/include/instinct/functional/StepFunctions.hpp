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
    };

    using GenericChain = BaseRunnable<JSONContextPtr, JSONContextPtr>;
    using GenericChainPtr = RunnablePtr<JSONContextPtr, JSONContextPtr>;

    class BaseStepFunction : public virtual IStepFunction, public GenericChain {
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
//
//        [[nodiscard]] std::vector<std::string> GetInputKeys() const override {
//            return steps_.front()->GetInputKeys();
//        }
//
//        [[nodiscard]] std::vector<std::string> GetOutputKeys() const override {
//            return steps_.front()->GetOutputKeys();
//        }
    };

    class MappingStepFunction final : public BaseStepFunction {
        std::unordered_map<std::string, StepFunctionPtr> steps_{};
    public:
        using MapDataType = nlohmann::json;


        explicit MappingStepFunction(std::unordered_map<std::string, StepFunctionPtr> steps)
                : steps_(std::move(steps)) {
            assert_true(!steps_.empty(), "Steps cannot be empty");
        }

        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            JSONMappingContext mapping_data;
            for (const auto &[k, v]: steps_) {
                // context should be copied for child steps
                JSONContextPtr new_ctx = CloneJSONContext(input);
                mapping_data[k] = v->Invoke(new_ctx);
            }
            input->ProduceMappingData(mapping_data);
            return input;
        }

    };

    class PassthroughStepFunction: public BaseStepFunction {
    public:
        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            return input;
        }
    };

    class PickingStepFunction: public BaseStepFunction {

    };

    class SelectionStepFunction final: public BaseStepFunction {
        std::string variable_name_;

    public:
        explicit SelectionStepFunction(std::string variable_name)
            : variable_name_(std::move(variable_name)) {
        }

        JSONContextPtr Invoke(const JSONContextPtr& input) override {
            auto mapping_data = input->RequireMappingData();
            return mapping_data[variable_name_];
        }
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


        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            return std::invoke(fn_, input);
        }
    };

    class BranchStepFunction final: public BaseStepFunction {
        StepFunctionPtr condition_;
        StepFunctionPtr branch_a_;
        StepFunctionPtr branch_b_;

    public:
        BranchStepFunction(StepFunctionPtr condition, StepFunctionPtr branch_a, StepFunctionPtr branch_b)
            : condition_(std::move(condition)),
              branch_a_(std::move(branch_a)),
              branch_b_(std::move(branch_b)) {
        }

        BranchStepFunction(const StepLambda& condition, StepFunctionPtr branch_a, StepFunctionPtr branch_b)
            : BranchStepFunction(std::make_shared<LambdaStepFunction>(condition), std::move(branch_a), std::move(branch_b)) {
        }

        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            if(const auto result = condition_->Invoke(input); result->RequirePrimitive<bool>()) {
                return branch_a_->Invoke(input);
            }
            return branch_b_->Invoke(input);
        }
    };

}


#endif //INSTINCT_STEPFUNCTIONS_HPP
