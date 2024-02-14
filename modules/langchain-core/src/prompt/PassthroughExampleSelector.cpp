//
// Created by RobinQu on 2024/2/14.
//

#include "PassthroughExampleSelector.h"

namespace LC_CORE_NS {
    std::vector<OptionDict> PassthroughExampleSelector::SelectExamples(const OptionDict& variables) {
        return examples_;
    }
} // LC_CORE_NS