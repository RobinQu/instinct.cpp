//
// Created by RobinQu on 2024/2/14.
//

#include "MutableExampleSelector.h"

namespace LC_CORE_NS {
    void MutableExampleSelector::AddExample(const OptionDict& example) {
        examples_.push_back(example);
    }

    const std::vector<OptionDict>& MutableExampleSelector::GetAllExamples() const {
        return examples_;
    }
} // LC_CORE_NS