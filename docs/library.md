# Getting started with `instinct.cpp` library

## Install from package manager

WIP [#1](https://github.com/RobinQu/instinct.cpp/issues/1)

## Build from sources

System Requirements:

* CMake 3.26+
* Compiler that supports C++ 20: GCC 12+ or Clang 15+
* Conan 2+

This project relies on [conan](https://conan.io/) to resolve dependencies. To build and install:

```shell
conan install . --build=missing --output-folder=build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
```


## Quick start

Let's build a simple text completion task using Ollama API.

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


## What's next

You can learn more about this frameworks by following links below:

* [Chaining](./chaining.md)
* [Built-in components](./components.md)
* Read more about single-binary services
    * [doc-agent](../modules/instinct-examples/doc-agent/README.md) : Chat with your docs locally with privacy.
    * [mini-assistant-api](../modules/instinct-examples/mini-assistant/README.md): Fully compatible with OpenAI's Assistant API at your service locally.

