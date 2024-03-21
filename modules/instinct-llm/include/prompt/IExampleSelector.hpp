//
// Created by RobinQu on 2024/2/14.
//

#ifndef BASEEXMAPLESELECTOR_H
#define BASEEXMAPLESELECTOR_H
#include "LLMGlobals.hpp"

#include "functional/JSONContextPolicy.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    using PromptExample = nlohmann::json;
    using PromptExamples = std::vector<PromptExample>;

    class IExampleSelector {
    public:
        virtual ~IExampleSelector() = default;

        virtual void AddExample(const PromptExample& example) = 0;

        virtual PromptExamples SelectExamples(const JSONContextPtr& variables) = 0;

        [[nodiscard]] virtual const PromptExamples& GetAllExamples() = 0;
    };

    using ExmapleSelectorPtr = std::shared_ptr<IExampleSelector>;
} // namespace INSTINCT_CORE_NS

#endif //BASEEXMAPLESELECTOR_H
