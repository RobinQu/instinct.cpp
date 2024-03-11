//
// Created by RobinQu on 2024/3/11.
//

#ifndef BASECHAIN_HPP
#define BASECHAIN_HPP

#include "IChain.hpp"
#include "IChainContextAware.hpp"
#include "LLMGlobals.hpp"

namespace INSTINCT_LLM_NS {
    struct ChainOptions {
        std::string input_prompt_key = "question";
        std::string output_answer_content_key = "answer_content";
        std::string output_ansewr_role_key = "answer_role";
    };

    template<typename Output, typename Options=ChainOptions>
    requires std::derived_from<Options, ChainOptions>
    class BaseChain: public IChain<Output>, public IChainContextAware {
        Options options_;
    public:
        explicit BaseChain(Options options = {})
            : options_(std::move(options)) {
        }

        const Options& GetOptions() {
            return options_;
        }

        Output Invoke(const LLMChainContext& input) override = 0;

        std::shared_ptr<ResultIterator<Output>> Batch(const std::vector<LLMChainContext>& input) override {
            auto view = input | std::views::transform([&](const auto& context) {
                return Invoke(context);
            });
            return create_result_itr_from_range(view);
        }

        std::shared_ptr<ResultIterator<Output>> Stream(const LLMChainContext& input) override {
            return create_result_itr_from_range({Invoke(input)});
        }

        std::vector<std::string> GetInputKeys() override {
            return {options_.input_prompt_key};
        }

        std::vector<std::string> GetOutputKeys() override {
            return {options_.output_ansewr_role_key, options_.output_answer_content_key};
        }
    };
}

#endif //BASECHAIN_HPP
