//
// Created by RobinQu on 2024/2/14.
//

#ifndef MUTABLEEXAMPLESELECTOR_H
#define MUTABLEEXAMPLESELECTOR_H

#include "BaseExmapleSelector.h"
#include "CoreGlobals.h"
#include "PromptTemplate.h"

namespace LC_CORE_NS {

class MutableExampleSelector: public BaseExmapleSelector{
protected:
    std::vector<OptionDict> examples_;
    PromptTemplatePtr example_prompt_template_;

public:
    explicit MutableExampleSelector(std::vector<OptionDict> examples, PromptTemplatePtr prompt_template)
        : examples_(std::move(examples)), example_prompt_template_(std::move(prompt_template)) {
    }
    void AddExample(const OptionDict& example) override;

    [[nodiscard]] const std::vector<OptionDict>& GetAllExamples() const override;
};

} // LC_CORE_NS

#endif //MUTABLEEXAMPLESELECTOR_H
