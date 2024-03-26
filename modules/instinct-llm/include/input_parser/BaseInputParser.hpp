//
// Created by RobinQu on 3/21/24.
//

#ifndef INSTINCT_BASEINPUTPARSER_HPP
#define INSTINCT_BASEINPUTPARSER_HPP

#include <utility>

#include "IInputParser.hpp"
#include "functional/BaseRunnable.hpp"
#include "functional/JSONContextPolicy.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    struct InputParserOptions {
//        std::string prompt_value_variable_key = DEFAULT_PROMPT_VARIABLE_KEY;
        std::string question_variable_key = DEFAULT_QUESTION_INPUT_OUTPUT_KEY;
    };

    /**
     * Responsible to write standard values to context, including:
     * 1. PromptValue message
     * 2. question string representing latest user input
     *
     * @tparam T
     */
    template<typename T>
    class BaseInputParser :
            public virtual IInputParser<T>,
            public virtual IConfigurable<InputParserOptions>,
            public BaseRunnable<T, JSONContextPtr> {
        InputParserOptions options_;
    public:
        explicit BaseInputParser(InputParserOptions options = {}) : options_(std::move(options)) {}

        [[nodiscard]] const InputParserOptions& GetOptions() const  {
            return options_;
        }
    public:
        JSONContextPtr Invoke(const T &input) override {
            return this->ParseInput(input);
        }


        void Configure(const InputParserOptions &options) override {
            options_ = options;
        }
    };

    template<typename T>
    using InputParserPtr = std::shared_ptr<BaseInputParser<T>>;
}

#endif //INSTINCT_BASEINPUTPARSER_HPP
