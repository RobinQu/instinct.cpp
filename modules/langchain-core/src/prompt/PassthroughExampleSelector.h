//
// Created by RobinQu on 2024/2/14.
//

#ifndef PASSTHROUGHEXAMPLESELECTOR_H
#define PASSTHROUGHEXAMPLESELECTOR_H

#include "CoreGlobals.h"
#include "MutableExampleSelector.h"

namespace LC_CORE_NS {

class PassthroughExampleSelector: public MutableExampleSelector {
public:
    PassthroughExampleSelector(std::vector<OptionDict> examples, PromptTemplatePtr prompt_template)
        : MutableExampleSelector(std::move(examples), std::move(prompt_template)) {
    }

    std::vector<OptionDict> SelectExamples(const OptionDict& variables) override;
};

} // LC_CORE_NS

#endif //PASSTHROUGHEXAMPLESELECTOR_H
