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
        std::string instruction_key_;

    public:
        explicit BaseOutputParser(std::string instruction_key = DEFAULT_FORMAT_INSTRUCTION_KEY)
            : instruction_key_(std::move(instruction_key)) {
        }

        std::string GetFormatInstruction() override = 0;

        /**
         * OtuputParser doesn't depend on any keys to output format instruction
         * @return
         */
        std::vector<std::string> GetInputKeys() override {
            return {};
        }

        void EnhanceContext(const ChainContextBuilderPtr& builder) override {
            builder->Put(instruction_key_, GetFormatInstruction());
        }


        std::vector<std::string> GetOutputKeys() override {
            return {instruction_key_};
        }

    };

    template<typename T>
    using OutputParserPtr = std::shared_ptr<BaseOutputParser<T>>;

}


#endif //BASEOUTPUTPARSER_HPP
