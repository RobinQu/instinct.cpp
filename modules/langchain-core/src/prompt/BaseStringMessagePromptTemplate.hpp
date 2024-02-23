//
// Created by RobinQu on 2024/2/22.
//


#ifndef BASESTRINGMESSAGEPROMPTTEMPLATE_HPP
#define BASESTRINGMESSAGEPROMPTTEMPLATE_HPP

#include "CoreGlobals.hpp"
#include "CoreTypes.hpp"
#include "MessagePromptTemplate.hpp"
#include "PlainPromptTemplate.hpp"
#include "StringPromptTemplate.hpp"


LC_CORE_NS {

    class BaseStringMessagePromptTemplate: public MessagePromptTemplate {
    protected:
        PlainPromptTemplate prompt_;

    public:
        explicit BaseStringMessagePromptTemplate(PlainPromptTemplate prompt)
            : prompt_(std::move(prompt)) {
        }

        std::vector<MessageVariant> FormatMessages(const OptionDict& variables) override;
    };

    inline std::vector<MessageVariant> BaseStringMessagePromptTemplate::FormatMessages(const OptionDict& variables) {
        return {Format(variables)};
    }
}

#endif //BASESTRINGMESSAGEPROMPTTEMPLATE_HPP
