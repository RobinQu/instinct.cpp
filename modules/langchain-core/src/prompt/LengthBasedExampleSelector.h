//
// Created by RobinQu on 2024/2/14.
//

#ifndef LENGTHBASEDPROMPTSELECTOR_H
#define LENGTHBASEDPROMPTSELECTOR_H

#include "BaseExmapleSelector.h"
#include "CoreGlobals.h"
#include "MutableExampleSelector.h"
#include "tools/StringUtils.h"

namespace LC_CORE_NS {

    namespace details {
        static auto DEFAULT_LENGTH_BASE_FUNCTION = [](const std::string& str) {
            return langchian::core::StringUtils::Resplit(str).size();
        };
        static int DEFAULT_MAX_LENGTH = 25;
    }

class LengthBasedExampleSelector: public MutableExampleSelector{

    int max_length_;
    std::function<size_t(const std::string&)> length_function_;
    std::vector<size_t> lengths_;

public:
    LengthBasedExampleSelector(std::vector<OptionDict> examples, PromptTemplatePtr prompt_template, int max_length = details::DEFAULT_MAX_LENGTH,
        std::function<size_t(const std::string&)> length_function = details::DEFAULT_LENGTH_BASE_FUNCTION);

    std::vector<OptionDict> SelectExamples(const OptionDict& variables) override;

    void AddExample(const OptionDict& example) override;
};

} // LC_CORE_NS

#endif //LENGTHBASEDPROMPTSELECTOR_H
