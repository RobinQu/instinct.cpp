//
// Created by RobinQu on 3/22/24.
//

#ifndef INSTINCT_MESSAGECHAIN_HPP
#define INSTINCT_MESSAGECHAIN_HPP

#include <utility>

#include <instinct/functional/runnable_chain.hpp>
#include <instinct/input_parser/base_input_parser.hpp>
#include <instinct/output_parser/base_output_parser.hpp>
#include <instinct/model/language_model.hpp>
#include <instinct/functional/xn.hpp>

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    template<
            typename Input,
            typename Output = JSONContextPtr,
            typename InputParser = InputParserPtr<Input>
    >
    class LeftHandPartialMessageChain final: public BaseRunnable<Input, Output>{
        InputParser input_parser_;
        StepFunctionPtr step_;

    public:
        [[nodiscard]] InputParser GetInputParser() const {
            return input_parser_;
        }

        [[nodiscard]] StepFunctionPtr GetStepFunction() const {
            return step_;
        }

        LeftHandPartialMessageChain(const InputParser &input_parser, const StepFunctionPtr& step)
            : input_parser_(input_parser),
              step_(step) {
        }

        Output Invoke(const Input &input) override {
            auto result = input_parser_->Invoke(input);
            return step_->Invoke(result);
        }
    };

    template<
            typename Output,
            typename Input = JSONContextPtr,
            typename OutputParser = OutputParserPtr<Output>
    >
    class RightHandPartialMessageChain final: public BaseRunnable<Input, Output> {
        OutputParser output_parser_;
        StepFunctionPtr step_;

    public:
        [[nodiscard]] OutputParser GetOutputParser() const {
            return output_parser_;
        }

        [[nodiscard]] StepFunctionPtr GetStepFunction() const {
            return step_;
        }

        RightHandPartialMessageChain(const OutputParser &output_parser, const StepFunctionPtr &step)
            : output_parser_(output_parser),
              step_(step) {
        }

        Output Invoke(const Input &input) override {
            auto result = step_->Invoke(input);
            return output_parser_->Invoke(result);
        }
    };


    /**
     * MessageChain is kind of one-path runnable. Underlying `StepFunction` always expects a string prompt and returns a `Generation`, so that it works with `InputParser` and `OutputParser`.
     * @tparam Input
     * @tparam Output
     * @tparam InputParser
     * @tparam OutputParser
     */
    template<
            typename Input,
            typename Output,
            typename InputParser = InputParserPtr<Input>,
            typename OutputParser = OutputParserPtr<Output>
    >
    class MessageChain : public BaseRunnable<Input, Output> {
    protected:
        InputParser input_parser_;
        OutputParser output_parser_;
        ChainOptions options_;
    public:
        explicit MessageChain(
                const InputParserPtr<Input> &input_parser,
                const OutputParserPtr<Output> &output_parser,
                const ChainOptions &options = {}) :
                BaseRunnable<Input, Output>(),
                input_parser_(input_parser),
                output_parser_(output_parser),
                options_(options) {
            assert_true(input_parser_, "should have InputParser");
            assert_true(output_parser_, "should have OutputParser");
        }

        [[nodiscard]] const ChainOptions &GetOptions() const {
            return options_;
        }

        [[nodiscard]] virtual StepFunctionPtr GetStepFunction() const = 0;

        OutputParserPtr<Output> GetOutputParser() const {
            return output_parser_;
        }

        InputParserPtr<Input> GetInputParser() const {
            return input_parser_;
        }

        Output Invoke(const Input &input) override {
            auto context = input_parser_->Invoke(input);
            auto step = GetStepFunction();
            auto output = step->Invoke(context);
            return output_parser_->Invoke(output);
        }
    };

    template<
            typename Input,
            typename Output
    >
    class FunctionalMessageChain final : public MessageChain<Input, Output> {
        StepFunctionPtr step_function_;
    public:
        FunctionalMessageChain(
                const InputParserPtr<Input> &input_parser,
                const OutputParserPtr<Output> &output_parser,
                StepFunctionPtr  step_function,
                const ChainOptions &options = {}) :
                MessageChain<Input,Output>(input_parser,
                                           output_parser,
                                           options),
                step_function_(std::move(step_function)) {}

        [[nodiscard]] StepFunctionPtr GetStepFunction() const override {
            return step_function_;
        }
    };

    /**
     * universal pointer for all chains
     */
    template<typename Input, typename Output>
    using MessageChainPtr = std::shared_ptr<MessageChain<Input, Output>>;

    template<typename Input, typename Output>
    MessageChainPtr<Input,Output> CreateFunctionalChain(
            const InputParserPtr<Input>& input_parser,
            const OutputParserPtr<Output>& output_parser,
            StepFunctionPtr step_function,
            const ChainOptions &options = {}
            ) {
        return std::make_shared<FunctionalMessageChain<Input,Output>>(input_parser, output_parser, step_function, options);
    }

    /**
     *  simple chain with variant input and string output
     */
    using TextChain = MessageChain<PromptValueVariant, std::string>;

//    class TextChain: public BaseRunnable<PromptValueVariant, std::string> {

//    };

    /**
     * pointer to TextChain
     */
    using TextChainPtr = std::shared_ptr<TextChain>;

    /**
     * Simple chain with variant input and multi-line text output
     */
    using MultilineChain = MessageChain<PromptValueVariant, MultilineGeneration>;

//    class MultilineChain: public BaseRunnable<PromptValueVariant, MultilineGeneration> {
//
//    };

    /**
     * Pointer to MultilineChain
     */
    using MultilineChainPtr = std::shared_ptr<MultilineChain>;


}


namespace xn::steps {
    using namespace INSTINCT_CORE_NS;
    using namespace INSTINCT_LLM_NS;

    class GenerationToStringFunction final: public BaseStepFunction {
    public:
        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            const auto generation = input->RequireMessage<Generation>();
            input->ProducePrimitive(generation.has_message() ? generation.message().content() : generation.text());
            return input;
        }
    };

    class CombineMessageList final: public BaseStepFunction {
    public:
        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            auto message_list = input->RequireMessage<MessageList>();
            input->ProducePrimitive(MessageUtils::CombineMessages(message_list.messages()));
            return input;
        }
    };

    static StepFunctionPtr stringify_generation() {
        return std::make_shared<GenerationToStringFunction>();
    }

    static StepFunctionPtr combine_chat_history() {
        return std::make_shared<CombineMessageList>();
    }
}


template <typename Input>
inline std::shared_ptr<INSTINCT_LLM_NS::LeftHandPartialMessageChain<Input>> operator|(const INSTINCT_LLM_NS::InputParserPtr<Input>& input_parser, const INSTINCT_LLM_NS::StepFunctionPtr& step_function) {
    return std::make_shared<INSTINCT_LLM_NS::LeftHandPartialMessageChain<Input>>(input_parser, step_function);
}

template <typename Input, typename Output>
inline INSTINCT_LLM_NS::MessageChainPtr<Input, Output> operator|(const std::shared_ptr<INSTINCT_LLM_NS::LeftHandPartialMessageChain<Input>>& left_hand_part, const INSTINCT_LLM_NS::OutputParserPtr<Output>& output_parser) {
    return std::make_shared<INSTINCT_LLM_NS::FunctionalMessageChain<Input,Output>>(left_hand_part->GetInputParser(), output_parser, left_hand_part->GetStepFunction());
}

template <typename Input>
inline std::shared_ptr<INSTINCT_LLM_NS::LeftHandPartialMessageChain<Input>> operator|(const std::shared_ptr<INSTINCT_LLM_NS::LeftHandPartialMessageChain<Input>>& left_hand_part, const INSTINCT_LLM_NS::StepFunctionPtr& next_step) {
    return std::make_shared<INSTINCT_LLM_NS::LeftHandPartialMessageChain<Input>>(
        left_hand_part->GetInputParser(),
        left_hand_part->GetStepFunction() | next_step
        );
}

#endif //INSTINCT_MESSAGECHAIN_HPP
