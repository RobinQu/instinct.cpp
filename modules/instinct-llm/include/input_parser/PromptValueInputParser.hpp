//
// Created by RobinQu on 3/21/24.
//

#ifndef INSTINCT_PROMPTVALUEINPUTPARSER_HPP
#define INSTINCT_PROMPTVALUEINPUTPARSER_HPP


#include "BaseInputParser.hpp"
#include "model/ILanguageModel.hpp"

namespace INSTINCT_LLM_NS {
    class PromptValueInputParser final: public BaseInputParser<PromptValue> {
    public:

        /**
         * passthrough implementation for PromptValue
         * @param input
         * @return
         */
        JSONContextPtr ParseInput(const PromptValue &input) override {
            auto context = CreateJSONContext();
            context->PutMessage(DEFAULT_PROMPT_INPUT_KEY, input);
            return context;
        }
    };
}



#endif //INSTINCT_PROMPTVALUEINPUTPARSER_HPP
