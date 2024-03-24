//
// Created by RobinQu on 3/23/24.
//

#ifndef INSTINCT_PROMPTVALUEVARIANTINPUTPARSER_HPP
#define INSTINCT_PROMPTVALUEVARIANTINPUTPARSER_HPP


#include "BaseInputParser.hpp"
#include "model/ILanguageModel.hpp"

namespace INSTINCT_LLM_NS {

    class PromptValueVariantInputParser : public BaseInputParser<PromptValueVariant> {

    public:
        JSONContextPtr ParseInput(const PromptValueVariant &input) override {
            return CreateJSONContext({
                {
                    GetOptions().question_variable_key,
                    details::conv_prompt_value_variant_to_string(input)
                }
             });
        }
    };
}

#endif //INSTINCT_PROMPTVALUEVARIANTINPUTPARSER_HPP
