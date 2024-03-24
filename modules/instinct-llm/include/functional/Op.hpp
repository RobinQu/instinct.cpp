//
// Created by RobinQu on 2024/3/24.
//

#ifndef XN_HPP
#define XN_HPP


#include "LLMGlobals.hpp"
#include "functional/StepFunctions.hpp"
#include "functional/JSONContextPolicy.hpp"

namespace xn::ops {
    using namespace INSTINCT_CORE_NS;
    using namespace INSTINCT_LLM_NS;

    class GenerationToStringFunction final: public BaseStepFunction {
    public:
        [[nodiscard]] std::vector<std::string> GetInputKeys() const override {
            return {};
        }

        [[nodiscard]] std::vector<std::string> GetOutputKeys() const override {
            return {};
        }

    public:
        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            auto generation = input->RequireMessage<Generation>();
            input->ProducePrimitive(generation.has_message() ? generation.message().content() : generation.text());
            return input;
        }
    };

    static StepFunctionPtr stringify_generation() {
        return std::make_shared<GenerationToStringFunction>();
    }
}




#endif //XN_HPP
