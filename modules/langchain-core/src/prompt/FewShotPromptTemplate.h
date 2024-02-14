//
// Created by RobinQu on 2024/1/10.
//

#ifndef FEWSHOTPROMPTTEMPLATE_H
#define FEWSHOTPROMPTTEMPLATE_H

#include "BasePromptTemplate.h"
#include <string>
#include <vector>

#include "BaseExmapleSelector.h"
#include "CoreGlobals.h"
#include "PromptTemplate.h"


namespace LC_CORE_NS {

    namespace details {
        static const auto DEFAULT_EXAMPLE_SEPERATOR = "\n\n";
    }

class FewShotPromptTemplate: public StringPromptTemplate {

    BaseExampleSelectorPtr example_selector_;
    PromptTemplatePtr example_prompt_template_;
    std::string prefix_;
    std::string suffix_;
    std::string example_seperator_;

public:
    FewShotPromptTemplate(BaseExampleSelectorPtr example_selector, PromptTemplatePtr example_prompt_template,
        std::string prefix, std::string suffix, std::string example_seperator = details::DEFAULT_EXAMPLE_SEPERATOR)
        : example_selector_(std::move(example_selector)),
          example_prompt_template_(std::move(example_prompt_template)),
          prefix_(std::move(prefix)),
          suffix_(std::move(suffix)),
        example_seperator_(std::move(example_seperator)){
    }

    std::string Format() override;

    std::string Format(const OptionDict& variables) override;
};

} // core
// langchain

#endif //FEWSHOTPROMPTTEMPLATE_H
