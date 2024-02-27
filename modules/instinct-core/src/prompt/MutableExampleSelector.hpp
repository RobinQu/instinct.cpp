//
// Created by RobinQu on 2024/2/14.
//

#ifndef MUTABLEEXAMPLESELECTOR_H
#define MUTABLEEXAMPLESELECTOR_H

#include "CoreGlobals.hpp"
#include "ExampleSelector.hpp"
#include "PlainPromptTemplate.hpp"

namespace INSTINCT_CORE_NS {
    class MutableExampleSelector : public ExampleSelector {
    protected:
        std::vector<OptionDict> examples_;
        PlainPromptTemplate example_prompt_template_;

    public:
        MutableExampleSelector(std::vector<OptionDict> examples, PlainPromptTemplate prompt_template)
            : examples_(std::move(examples)), example_prompt_template_(std::move(prompt_template)) {
        }

        void AddExample(const OptionDict& example) override;

        [[nodiscard]] const std::vector<OptionDict>& GetAllExamples() const override;
    };


    inline void MutableExampleSelector::AddExample(const OptionDict& example) {
        examples_.push_back(example);
    }

    inline const std::vector<OptionDict>& MutableExampleSelector::GetAllExamples() const {
        return examples_;
    }
} // namespace INSTINCT_CORE_NS

#endif //MUTABLEEXAMPLESELECTOR_H
