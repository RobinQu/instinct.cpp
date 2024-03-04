//
// Created by RobinQu on 2024/2/14.
//

#ifndef LENGTHBASEDPROMPTSELECTOR_H
#define LENGTHBASEDPROMPTSELECTOR_H

#include "CoreGlobals.hpp"
#include "CoreTypes.hpp"
#include "MutableExampleSelector.hpp"
#include "tools/StringUtils.hpp"

namespace INSTINCT_CORE_NS {

    namespace details {
        static auto DEFAULT_LENGTH_BASE_FUNCTION = [](const std::string& str) {
            return StringUtils::Resplit(str).size();
        };
        static int DEFAULT_MAX_LENGTH = 25;
    }

class LengthBasedExampleSelector: public MutableExampleSelector{

    int max_length_;
    std::function<size_t(const std::string&)> length_function_;
    std::vector<size_t> lengths_;

public:
    LengthBasedExampleSelector(std::vector<OptionDict> examples, PlainPromptTemplate prompt_template, int max_length = details::DEFAULT_MAX_LENGTH,
        std::function<size_t(const std::string&)> length_function = details::DEFAULT_LENGTH_BASE_FUNCTION);

    std::vector<OptionDict> SelectExamples(const OptionDict& variables) override;

    void AddExample(const OptionDict& example) override;
};

    inline LengthBasedExampleSelector::LengthBasedExampleSelector(std::vector<OptionDict> examples, PlainPromptTemplate prompt_template, const int max_length,
                                                           std::function<size_t(const std::string&)> length_function)
            : MutableExampleSelector(std::move(examples), std::move(prompt_template)),
              max_length_(max_length),
              length_function_(std::move(length_function)),
    lengths_(){
    }

    inline void LengthBasedExampleSelector::AddExample(const OptionDict& example) {
        MutableExampleSelector::AddExample(example);
        lengths_.push_back(example_prompt_template_.Format(example).size());
    }

    inline std::vector<OptionDict> LengthBasedExampleSelector::SelectExamples(const OptionDict& variables) {
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

} // namespace INSTINCT_CORE_NS

#endif //LENGTHBASEDPROMPTSELECTOR_H
