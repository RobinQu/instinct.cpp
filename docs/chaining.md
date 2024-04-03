# Functional chaining in `instinct.cpp`

Chaining gets popular these days, as `ranges` is available in C++ 20.

Let's look at a code snippet to process numbers. 

```c++
#include <iostream>
#include <ranges>
 
int main()
{
    auto even = [](int i) { return 0 == i % 2; };
    auto square = [](int i) { return i * i; };
 
    for (int i : std::views::iota(0, 6)
               | std::views::filter(even)
               | std::views::transform(square))
        std::cout << i << ' ';
    std::cout << '\n';
}
```

_Credits to [cppreference.com](https://en.cppreference.com/w/cpp/ranges/filter_view)._

In the example above:

* a non-owning source is created by `std::vies::iota`.
* two operations are applied for each data emitted from source.
* operations are concat using bitwise OR operator `|`, which is called pipe operator as well. This is kind of (good) abuse of bitwise operator as it is overridden to compose a sequence of operations other than manipulating bits data.

The whole processing flow is direct and easy to understand. And in `instinct.cpp`, such programing techniques are also employed to compose complex LLM-based pipeline.


## `StepFunction` and their friends


In `quick_start.cpp`, we have built a sequential chain:

```c++
#include "chain/MessageChain.hpp"
#include "input_parser/PromptValueVariantInputParser.hpp"
#include "chat_model/OllamaChat.hpp"
#include "output_parser/StringOutputParser.hpp"
#include "prompt/PlainPromptTemplate.hpp"

int main() {
    using namespace INSTINCT_CORE_NS;
    using namespace INSTINCT_LLM_NS;

    const auto input_parser = CreatePromptVariantInputParser();
    const auto string_prompt = CreatePlainPromptTemplate("Answer following question in one sentence: {question}");
    const auto output_parser = CreateStringOutputParser();
    const auto chat_model = CreateOllamaChatModel();
    const auto xn = input_parser | string_prompt |  chat_model->AsModelFunction() | output_parser;
    const auto result = xn->Invoke("Why sky is blue?");
    std::cout << result <<std::endl;
}

```

In fact, all sub-classes of [StepFunction](../modules/instinct-core/include/functional/StepFunctions.hpp) are all composable. `StepFunction`s have many desirable characteristics:

* They share same input type and same output type. That's `JSONContextPtr`.
* They share similar functional interface. That's `Invoke(T)`, `Batch(std::vector<T>)` and  `Stream(T)`. 
* And most awesome part: they can be concatenated together.

Many of you many find `input_parser` and `output_parser` are not `StepFunction`. They are special adaptors that convert user-defined type `T` to `JSONContextPtr` and vice versa.


## Access to `JSONContext`

Most implementation of `StepFunction`s are reading and writing a `JSONContext`. They are reading values from `JSONContext` as arguments of ongoing call, and writing something back to `JSONContext` as return value.

Please refer to `instinct::core::IContext` interface for more details. If you are writing your own `StepFunction`, this may be crucial.

## Go parallel

Many pipelines involve sub-path or multi-path execution. And it can be done with the help of `MappingStepFunction`.

Let's see a code snippet from `LLMChain.cpp`.

```c++
auto context_fn = xn::steps::mapping({
                {"format_instruction", output_parser->AsInstructorFunction()},
                {"chat_history", chat_memory->AsLoadMemoryFunction()},
                {"question", xn::steps::passthrough()}
            });
auto step_function =  xn::steps::mapping({
        {"answer", context_fn | prompt_template | model_function},
        {"question", xn::steps::selection("question")}
    })
    | chat_memory->AsSaveMemoryFunction({.is_question_string=true, prompt_variable_key="question", .answer_variable_key="answer"})
    | xn::steps::selection("answer");
```

Step by step illustrations:

* `xn::steps::mapping` is a factory method to create a shared pointer to `MappingStepFunction`.
* `MappingStepFunction` takes an `std::unordered_map<std::string, StepFunctionPtr>` as argument, which declares all the forking sub-paths.
* Input data to `MappingStepFunction` will be copied and forward as argument to sub-path `StepFunction`.
* A `JSONMappingContext` will be produced by `MappingStepFunction` for downstream. So the following `StepFunction` should expect `JSONMappingContext` in the data context.
* Luckily, `prompt_template` is kind of `StepFunction` that expects a  `JSONMappingContext` as input, which will be used as template variables to render a text prompt.
* Memory function of `chat_memory` is also expecting a `JSONMappingContext`, but it will ask about which entries should be treated as `question` and `answer` for persistence.
* Lastly,  `xn::steps::selection` is kind of map reducer that will convert a `JSONMappingContext` to primitive value like `int`. Here, we select the value of entry `answer` as reduced value. In this pipeline, this should be string value of model output. 

You can find more complex pipelines with mapping function in  `RAGChain.cpp`.

## What's next

There are many built-in components that can be used to compose your LLM pipelines. Please check out [components.md](./components.md) for overviews.
