//
// Created by RobinQu on 2024/3/11.
//

#ifndef BASEOUTPUTPARSER_HPP
#define BASEOUTPUTPARSER_HPP

#include "IOutputParser.hpp"
#include "LLMGlobals.hpp"
#include "chain/IChainContextAware.hpp"

namespace INSTINCT_LLM_NS {
    static std::string DEFAULT_FORMAT_INSTRUCTION_KEY = "format_instruction";

    template<typename T>
    class BaseOutputParser: public IOutputParser<T>, public IChainContextAware {
    public:

        std::string GetFormatInstruction() override = 0;

        std::vector<std::string> GetInputKeys() override {
            return {DEFAULT_FORMAT_INSTRUCTION_KEY};
        }

        void EnhanceContext(const ChainContextBuilderPtr& builder) override {
            builder->Put(DEFAULT_FORMAT_INSTRUCTION_KEY, GetFormatInstruction());
        }


    };

    template<typename T>
    using OutputParserPtr = std::shared_ptr<BaseOutputParser<T>>;

}


#endif //BASEOUTPUTPARSER_HPP
