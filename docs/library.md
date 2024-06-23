# Getting started with `instinct.cpp` library

## Install from package manager

WIP [#1](https://github.com/RobinQu/instinct.cpp/issues/1)

## Build from sources

Build requirements:

* CMake 3.26+
* Compiler that supports C++ 20: GCC 12+ or Clang 15+
* Conan 2+ (Optional)

### Option 1. Using `conan`
If you are using conan to resolve all dependencies, simple run the `install` command:

```shell
conan install conanfile.py --build=missing --output-folder=build
```

To build and install to `/tmp/instinct.cpp`:

```shell

cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/tmp/instinct.cpp
cmake --build . -j $(nproc) --target install
```

### Option 2. Using `CMake` only

Some dependencies are expected to be installed in system, and you should make sure they can be found by Cmake's `find_package` command. These libraries include:

* protobuf >= 5.27.0, which brings in `absl` and `googletest` transitively.
* ICU >= 74.1

For example, on macOS you can install them using `brew`: 

```shell
brew install protobuf icu4c
```

However, It's recommended to build and install these libraries from source because of following reasons:

1. You can make sure both static and dynamic libraries are available. Some package managers like `brew` only ship dynamic libraries of protobuf so that it may prevent you from linking statically.
2. Mainstream providers like `apt` and `yum` are shipping very old versions of these libraries in "stable" channel. 

I know it's possible to build a standalone `protobuf` using Cmake's `FetchContent`. But I haven't figured a proper way to make every pieces working together.  

And then just build and install in common practices. Example to build install to `/tmp/instinct.cpp` with default project settings.

```shell
cd build
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/tmp/instinct.cpp
cmake --build build -j $(nproc) --config Release --target install
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

You can learn more about frameworks by following links below:

* [Chaining](./chaining.md)
* [Built-in components](./components.md)
* Read more about single-binary services
    * [doc-agent](../modules/instinct-examples/doc-agent/README.md) : Chat with your docs locally with privacy.
    * [mini-assistant-api](../modules/instinct-examples/mini-assistant/README.md): Fully compatible with OpenAI's Assistant API at your service locally.
* [Testing](./testing.md)
