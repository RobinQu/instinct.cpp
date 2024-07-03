# Getting started with `instinct.cpp` library

## Install from package manager

WIP [#1](https://github.com/RobinQu/instinct.cpp/issues/1)

## Build from sources

Build requirements:

* CMake 3.26+
* Compiler that supports C++ 20: GCC 12+ or Clang 15+
* Conan 2+ (Optional)

Simple run the `install` command:

```shell
conan install conanfile.py --build=missing --output-folder=build
```

As I am still working on publishing this package to conan center, you have to install to local `conan` cache:

```
conan create . 
```

## Quick start

In your project's `conanfile.py`, add `instinct-cpp` as requirement. A working example is hosted [here](https://github.com/RobinQu/instinct-cpp-examples/tree/master/quick_start_simple).

```py
from conan import ConanFile
from conan.tools.build import check_min_cppstd

class YourRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    
    def validate(self):
        check_min_cppstd(self, 20)

    def requirements(self):
        # toggle off all switches
        # you can turn on for further experiments
        self.requires("instinct-cpp/0.1.5")
```

And prepare dependencies:

```shell
conan install conanfile.py --build=missing
```

Once installed, let's build a simple text completion task using Ollama API.

```c++
#include <instinct/chain/message_chain.hpp>
#include <instinct/input_parser/PromptValueVariantInputParser.hpp>
#include <instinct/chat_model/ollama_chat.hpp>
#include <instinct/output_parser/string_output_parser.hpp>
#include <instinct/prompt/plain_prompt_template.hpp>

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

You can learn more about frameworks by following links below:

* [Chaining](./chaining.md)
* [Built-in components](./components.md)
* Read more about single-binary services
    * [doc-agent](../modules/instinct-apps/doc-agent/README.md) : Chat with your docs locally with privacy.
    * [mini-assistant-api](../modules/instinct-apps/mini-assistant/README.md): Fully compatible with OpenAI's Assistant API at your service locally.
* [Testing](./testing.md)
