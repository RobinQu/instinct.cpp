//
// Created by RobinQu on 2024/2/14.
//

#ifndef PASSTHROUGHEXAMPLESELECTOR_H
#define PASSTHROUGHEXAMPLESELECTOR_H

#include "CoreGlobals.hpp"
#include "MutableExampleSelector.hpp"

namespace INSTINCT_CORE_NS {
    class PassthroughExampleSelector : public MutableExampleSelector {
    public:
        PassthroughExampleSelector(std::vector<OptionDict> examples, PlainPromptTemplate prompt_template)
            : MutableExampleSelector(std::move(examples), std::move(prompt_template)) {
        }

        std::vector<OptionDict> SelectExamples(const OptionDict& variables) override;
    };

    inline std::vector<OptionDict> PassthroughExampleSelector::SelectExamples(const OptionDict& variables) {
        return GetAllExamples();
    }
} // namespace INSTINCT_CORE_NS

#endif //PASSTHROUGHEXAMPLESELECTOR_H
