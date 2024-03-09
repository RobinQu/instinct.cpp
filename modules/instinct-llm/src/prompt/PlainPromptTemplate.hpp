//
// Created by RobinQu on 2024/2/13.
//

#ifndef PROMPTTEMPLATE_H
#define PROMPTTEMPLATE_H
#include "LLMGlobals.hpp"
#include <fmt/format.h>
#include <fmt/args.h>

#include "StringPromptTemplate.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class PlainPromptTemplate : public StringPromptTemplate {
        std::string template_string_;

    public:
        // static PlainPromptTemplate FromTemplate(const std::string& template_string);

        explicit PlainPromptTemplate(std::string template_string)
            : StringPromptTemplate(), template_string_(std::move(template_string)) {
        }

        std::string Format(const LLMChainContext& variables) override {
            fmt::dynamic_format_arg_store<fmt::format_context> store;
            // assuming `variables` has depth of one

            for(const auto& [k,v]: variables.values()) {
                switch (v.value_case()) {
                    case PrimitiveVariable::kIntValue:
                        store.push_back(fmt::arg(k.c_str(), v.int_value()));
                        break;
                    case PrimitiveVariable::kLongValue:
                        store.push_back(fmt::arg(k.c_str(), v.long_value()));
                        break;
                    case PrimitiveVariable::kFloatValue:
                        store.push_back(fmt::arg(k.c_str(), v.float_value()));
                        break;
                    case PrimitiveVariable::kDoubleValue:
                        store.push_back(fmt::arg(k.c_str(), v.double_value()));
                        break;
                    case PrimitiveVariable::kBoolValue:
                        store.push_back(fmt::arg(k.c_str(), v.bool_value()));
                        break;
                    case PrimitiveVariable::kStringValue:
                        store.push_back(fmt::arg(k.c_str(), v.string_value().c_str()));
                        break;
                    default:
                        throw InstinctException("unknown value type for entry: " + k);
                }
            }
            return fmt::vformat(this->template_string_, store);
        }
    };


} // namespace INSTINCT_CORE_NS

#endif //PROMPTTEMPLATE_H
