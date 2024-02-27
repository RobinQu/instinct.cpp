//
// Created by RobinQu on 2024/2/13.
//

#ifndef BASESTRINGMESSAGEPROMPTTEMPLATE_H
#define BASESTRINGMESSAGEPROMPTTEMPLATE_H
#include "Forwards.hpp"
#include "CoreGlobals.hpp"

namespace INSTINCT_CORE_NS {
    class MessagePromptTemplate {
    public:
        virtual ~MessagePromptTemplate() = default;

        virtual MessageVariant Format(const OptionDict& variables) = 0;

        virtual MessageVariant Format() {
            return Format({});
        }

        virtual std::vector<MessageVariant> FormatMessages(const OptionDict& variables) = 0;

        virtual std::vector<MessageVariant> FormatMessages() {
            return FormatMessages({});
        }
    };

} // namespace INSTINCT_CORE_NS

#endif //BASESTRINGMESSAGEPROMPTTEMPLATE_H
