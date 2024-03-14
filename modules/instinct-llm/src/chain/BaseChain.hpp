//
// Created by RobinQu on 2024/3/11.
//

#ifndef BASECHAIN_HPP
#define BASECHAIN_HPP

#include "IChain.hpp"
#include "IChainContextAware.hpp"
#include "LLMGlobals.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    struct ChainOptions {
        std::vector<std::string> input_keys {DEFAULT_PROMPT_INPUT_KEY};
        std::vector<std::string> output_keys  {DEFAULT_ANSWER_OUTPUT_KEY};
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

        Output Invoke(const ContextPtr& input) override = 0;

        AsyncIterator<Output> Batch(const std::vector<ContextPtr>& input) override {
            return rpp::source::from_iterable(input)
                | rpp::operators::map([&](const auto& context) {
                    return Invoke(context);
                });
        }

        AsyncIterator<Output> Stream(const ContextPtr& input) override {
            return rpp::source::from_callable([&]() {
                return Invoke(input);
            });
        }

        std::vector<std::string> GetInputKeys() override {
            return options_.input_keys;
        }

        std::vector<std::string> GetOutputKeys() override {
            return options_.output_keys;
        }
    };

    template<typename T, typename Options = ChainOptions>
    using ChainPtr = std::shared_ptr<BaseChain<T, Options>>;


    /**
     * Shared pointer to a TextChain
     */
    using TextChainPtr = ChainPtr<std::string>;

    /**
     * A Chain that generates a multi-line text result
     */
    using MultilineTextChainPtr = ChainPtr<std::vector<std::string>>;


}

#endif //BASECHAIN_HPP
