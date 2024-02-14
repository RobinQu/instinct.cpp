//
// Created by RobinQu on 2024/2/14.
//

#include "LengthBasedExampleSelector.h"

#include <utility>

#include "tools/StringUtils.h"

namespace LC_CORE_NS {


    LengthBasedExampleSelector::LengthBasedExampleSelector(std::vector<OptionDict> examples, PromptTemplatePtr prompt_template, const int max_length,
                                                           std::function<size_t(const std::string&)> length_function)
            : MutableExampleSelector(std::move(examples), std::move(prompt_template)),
              max_length_(max_length),
              length_function_(std::move(length_function)),
    lengths_(){
    }

    void LengthBasedExampleSelector::AddExample(const OptionDict& example) {
        MutableExampleSelector::AddExample(example);
        lengths_.push_back(example_prompt_template_->Format(example).size());
    }

    std::vector<OptionDict> LengthBasedExampleSelector::SelectExamples(const OptionDict& variables) {
        size_t remaining = max_length_;
        int i =0;
        const size_t size = examples_.size();
        std::vector<OptionDict> result;
        while(remaining>0 and i< size) {
            long len = 0L - lengths_[i] + remaining ;
            if(len<0) {
                break;
            }
            result.push_back(examples_[i]);
            i++;
        }
        return result;
    }
} // LC_CORE_NS