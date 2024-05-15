//
// Created by RobinQu on 2024/5/14.
//

#ifndef LLMCOMPILERJOINERTASKGRAPHINPUTPARSER_HPP
#define LLMCOMPILERJOINERTASKGRAPHINPUTPARSER_HPP
#include "LLMGlobals.hpp"
#include "TaskGraphUtils.hpp"
#include "input_parser/BaseInputParser.hpp"

namespace INSTINCT_LLM_NS {
    class LLMCompilerJoinerTaskGraphInputParser final: public BaseInputParser<LLMCompilerTaskGraph> {
    public:
        explicit LLMCompilerJoinerTaskGraphInputParser(InputParserOptions options)
            : BaseInputParser<LLMCompilerTaskGraph>(std::move(options)) {
        }

        JSONContextPtr ParseInput(const LLMCompilerTaskGraph &graph) override {
            std::string scratchpad;
            TaskGraphUtils::BuildAgentScrachPad(graph, scratchpad);
            return CreateJSONContext({
                {"question", graph.question()},
                {"agent_scrathpad", scratchpad},
                // TODO examples should contain tool usages, which are not defined at this stage
                {"examples", ""},
                {"messages", ""}
            });
        }
    };

    static InputParserPtr<LLMCompilerTaskGraph> CreateLLMCompilerJoinerTaskGraphInputParser(const InputParserOptions& options = {}) {
        return std::make_shared<LLMCompilerJoinerTaskGraphInputParser>(options);
    }

}
#endif //LLMCOMPILERJOINERTASKGRAPHINPUTPARSER_HPP
