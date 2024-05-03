//
// Created by RobinQu on 2024/1/12.
//

#ifndef BASELANGUAGEMODEL_H
#define BASELANGUAGEMODEL_H
#include <ranges>
#include <llm.pb.h>
#include "LLMGlobals.hpp"
#include "functional/IRunnable.hpp"
#include "prompt/MessageUtils.hpp"
#include "prompt/StringPromptTemplate.hpp"
#include "toolkit/BaseFunctionToolkit.hpp"
#include "tools/Assertions.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class ILanguageModel {
    public:
        ILanguageModel()=default;
        virtual ~ILanguageModel()=default;
        ILanguageModel(ILanguageModel&&)=delete;
        ILanguageModel(const ILanguageModel&)=delete;

        /**
         * Bind toolkits for LLM APIs that support tool uses. Only schema part of toolkit is used here.
         * @param toolkit
         */
        virtual void BindTools(const FunctionToolkitPtr& toolkit) {
            BindToolSchemas(toolkit->GetAllFuncitonToolSchema());
        }

        /**
         * Bind tool schemas for function tool calling APIs
         * @param function_tool_schema
         */
        virtual void BindToolSchemas(const std::vector<FunctionTool>& function_tool_schema) = 0;
    };

    namespace details {
        static auto visit_prompt_variant_as_string = overloaded {
            [](const StringPromptValue& spv) { return spv.text(); },
            [](const ChatPromptValue& cpv) { return MessageUtils::CombineMessages(cpv.messages()); },
            [](const PromptValue& pv) {
                if (pv.has_chat()) { return MessageUtils::CombineMessages(pv.chat().messages()); }
                if (pv.has_string()) {return pv.string().text();}
                throw InstinctException("PromptValue is empty");
            },
            [](const MessageList& message_list) { return MessageUtils::CombineMessages(message_list.messages()); },
            [](const Message& message) { return message.content(); },
            [](const std::string& str) {return str;}
        };

        static auto visit_prompt_variant_as_message_list = overloaded {
            [](const StringPromptValue& spv) {
                MessageList message_list;
                auto msg = message_list.add_messages();
                msg->set_content(spv.text());
                msg->set_role("user");
                return message_list;
            },
            [](const ChatPromptValue& cpv) {
                MessageList message_list;
                for (const auto&msg: cpv.messages()) {
                    message_list.add_messages()->CopyFrom(msg);
                }
                return message_list;
            },
            [](const PromptValue& pv) {
                MessageList message_list;
                if (pv.has_chat()) {
                    for (const auto&msg: pv.chat().messages()) {
                        message_list.add_messages()->CopyFrom(msg);
                    }
                }
                if (pv.has_string()) {
                    auto msg = message_list.add_messages();
                    msg->set_content(pv.string().text());
                    msg->set_role("user");
                }
                return message_list;
            },
            [](const MessageList& message_list) {
                return message_list;
            },
            [](const Message& message) {
                MessageList message_list;
                message_list.add_messages()->CopyFrom(message);
                return message_list;
            },
            [](const std::string& str) {
                MessageList message_list;
                const auto msg = message_list.add_messages();
                msg->set_content(str);
                msg->set_role("user");
                return  message_list;
            }
        };


        static MessageList conv_prompt_value_variant_to_message_list(const PromptValueVariant& pvv) {
            return std::visit(visit_prompt_variant_as_message_list, pvv);
        }

        static std::string conv_prompt_value_variant_to_string(const PromptValueVariant& pvv) {
            return std::visit(visit_prompt_variant_as_string, pvv);
        }

        static std::string conv_language_result_to_string(const  LangaugeModelResult& model_result) {
            assert_non_empty_range(model_result.generations());
            // std::cout << model_result.DebugString() << std::endl;
            const auto& gen = model_result.generations(0);
            if (gen.has_message()) {
                return gen.message().content();
            }
            return gen.text();
        }

        static Message conv_language_result_to_message(const LangaugeModelResult& model_result) {
            assert_non_empty_range(model_result.generations());
            const auto& gen = model_result.generations(0);
            assert_true(gen.has_message(), "should have message value in generation");
            return gen.message();
        }
    }

} // core
// langchian

#endif //BASELANGUAGEMODEL_H
