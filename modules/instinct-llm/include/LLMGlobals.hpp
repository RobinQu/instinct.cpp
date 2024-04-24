//
// Created by RobinQu on 2024/2/13.
//

#ifndef MODELGLOBALS_H
#define MODELGLOBALS_H


#include <llm.pb.h>
#include <agent.pb.h>
#include "CoreGlobals.hpp"
#include "tools/StringUtils.hpp"

#define INSTINCT_LLM_NS instinct::llm

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    using TokenSize = unsigned long;
    using TokenId = unsigned long;

    using Embedding = std::vector<float>;

    struct ChainOptions {

    };

    /**
     * LLM input variant
     */
    using PromptValueVariant = std::variant<
        StringPromptValue,
        ChatPromptValue,
        PromptValue,
        MessageList,
        Message,
        std::string
    >;


    /**
     * this should refer to a Generation
     */
    static const std::string DEFAULT_ANSWER_OUTPUT_KEY = "answer";


    /**
     * this should refer to a PromptValue
     */
    static const std::string DEFAULT_PROMPT_VARIABLE_KEY = "prompt";


    /**
     * this should refer to a string describing a question for retriever
     */
    static const std::string DEFAULT_QUESTION_INPUT_OUTPUT_KEY = "question";

    /**
     * this should refer to a string containing contextual information returned by retriever
     */
    static const std::string DEFAULT_CONTEXT_OUTPUT_KEY = "context";


    static const std::string DEFAULT_STANDALONE_QUESTION_INPUT_KEY = "standalone_question";


    using TemplateVariables = nlohmann::json;
    using TemplateVariablesPtr = std::shared_ptr<TemplateVariables>;

    using PromptExample = nlohmann::json;
    using PromptExamples = std::vector<PromptExample>;

    static TemplateVariablesPtr CreateTemplateVariable() {
        return std::make_shared<TemplateVariables>(nlohmann::json::parse("{}"));
    }



    inline std::ostream& operator<<(std::ostream& ostrm, const Message& msg) {
        ostrm << "Message[role=" << msg.role() << ", content=" << msg.content() << "]";
        return ostrm;
    }

    inline std::ostream& operator<<(std::ostream& ostrm, const Embedding& embedding) {
        ostrm << "Embedding[";
        for (const auto& f: embedding) {
            ostrm << f << ", ";
        }
        return ostrm << "]";
    }


    struct ModelOptions {
        std::vector<std::string> stop_words;
    };

    enum MessageRole {
        kUnknown = 0,
        kAsisstant,
        kHuman,
        kSystem,
        kFunction,
        kTool
    };

    using MessageRoleNameMapping = std::unordered_map<MessageRole, std::string>;

    /**
     * Default role name mapping. This follows convention of OpenAI API
     */
    static MessageRoleNameMapping DEFAULT_ROLE_NAME_MAPPING =  {
        {kAsisstant, "assistant"},
        {kHuman, "human"},
        {kSystem, "system"},
        {kFunction, "function"},
        {kTool, "tool"}
    };

    using ChatPrompTeplateLiterals = std::initializer_list<std::pair<MessageRole, std::string>>;

    static PromptValue CreatePromptValue(ChatPrompTeplateLiterals&& literals, const MessageRoleNameMapping& mapping = DEFAULT_ROLE_NAME_MAPPING) {
        PromptValue pv;
        const auto chat_prompt_value = pv.mutable_chat();
        for(const auto& [role,content]: literals) {
            auto* msg = chat_prompt_value->add_messages();
            msg->set_role(mapping.at(role));
            msg->set_content(content);
        }
        return pv;
    }


    /**
     * Render arguments of function tools according to JSON Schema.
     * @tparam T range container
     * @param args range of `FunctionToolArgument`
     * @return
     */
    template<typename T>
    requires RangeOf<T, FunctionToolArgument>
    static std::string RenderFunctionToolArgument(T&& args) {
        auto args_view = args | std::views::transform([](const FunctionToolArgument& arg) {
            // translate to python type which most LLM is more familar with
            // std::string arg_type_string;
            // const auto& pt = arg.type();
            // if (pt == INT32) arg_type_string = "int";
            // if (pt == INT64) arg_type_string = "int";
            // if (pt == FLOAT) arg_type_string = "float";
            // if (pt == DOUBLE) arg_type_string = "float";
            // if (pt == BOOL) arg_type_string = "bool";
            // if (pt == VARCHAR) arg_type_string = "string";
            return "\"" +  arg.name() + "\":\"" + arg.type() + "\"";
        });
        return  "{" + StringUtils::JoinWith(args_view, ",") + "}";
    }

    /**
     * Render function tool descriptions with given schema.
     * @tparam T
     * @param tools
     * @param with_args
     * @return
     */
    template<typename T>
    requires RangeOf<T, FunctionToolSchema>
    static std::string RenderFunctionTools(T&& tools, const bool with_args = true) {
        auto fn_desc_view = tools | std::views::transform([&](const FunctionToolSchema& fn_schema) {
            if (with_args) {
                return fmt::format("{}: {} args={}", fn_schema.name(), fn_schema.description(), RenderFunctionToolArgument(fn_schema.arguments()));
            }
            return fmt::format("{}: {}", fn_schema.name(), fn_schema.description());
        });
        return StringUtils::JoinWith(fn_desc_view, "\n");
    }

}


#endif //MODELGLOBALS_H
