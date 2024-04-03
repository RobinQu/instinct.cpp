//
// Created by RobinQu on 3/23/24.
//

#ifndef INSTINCT_PROMPTVALUEVARIANTINPUTPARSER_HPP
#define INSTINCT_PROMPTVALUEVARIANTINPUTPARSER_HPP


#include "BaseInputParser.hpp"
#include "model/ILanguageModel.hpp"

namespace INSTINCT_LLM_NS {

    class PromptValueVariantInputParser final: public BaseInputParser<PromptValueVariant> {
    public:

        JSONContextPtr ParseInput(const PromptValueVariant &input) override {
            auto ctx = CreateJSONContext();
            ctx->ProducePrimitive(details::conv_prompt_value_variant_to_string(input));
            return ctx;
        }
    };

    static InputParserPtr<PromptValueVariant> CreatePromptVariantInputParser() {
        return std::make_shared<PromptValueVariantInputParser>();
    }

}


#endif //INSTINCT_PROMPTVALUEVARIANTINPUTPARSER_HPP
