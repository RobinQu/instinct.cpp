# ✨ instinct.cpp

`instinct.cpp` is a toolkit for developing LLM-powered applications targeting edge computing.  

[![Discord](https://img.shields.io/badge/Discord%20Chat-purple?style=flat-square&logo=discord&logoColor=white&link=https%3A%2F%2Fdiscord.gg%2F5cVnVyh3)](https://discord.gg/5cVnVyh3)   [![C++ 20](https://img.shields.io/badge/C%2B%2B-20-blue?style=flat-square&link=https%3A%2F%2Fen.wikipedia.org%2Fwiki%2FC%252B%252B20)](https://en.wikipedia.org/wiki/C%2B%2B20)    [![License](https://img.shields.io/badge/Apache%20License-2.0-green?style=flat-square&logo=Apache&link=.%2FLICENSE)](./LICENSE)


**🚨 This project is under active development and has not reached to GA stage of first major release. See more at [Roadmap section](#roadmap).**

## Features

What `instinct.cpp` offer:

* Services that are working out-of-box.
  * Assistant API server: Agent service that is fully compatible with OpenAI's Assistant API.
    * [mini-assistant-api](./modules/instinct-examples/mini-assistant): Single binary for single node deployment with vector database and other dependencies bundled. 
    * `mighty-assistant-api`: (**WIP**) A cloud native implementation that is highly scalable with distributed components and multi-tenant support.  
  * [chat-agent](./modules/instinct-examples/doc-agent): A CLI application that create knowledge index with your docs (PDF,TXT,MD,...) and launch an HTTP server that is fully compatible with OpenAI `ChatCompletion`.
* Frameworks to build LLM-based applications. Say it `langchain.cpp`.   
  * Integration for privacy-first LLM providers: Built-in support for [Ollama](https://ollama.com/) and other OpenAI compatible API services like [nitro](https://nitro.jan.ai/) and more.
  * Building blocks for common application patterns like Chatbot, RAG, LLM Agent.
  * Functional chaining components for composable LLM pipelines.
  * Agent patterns: ReACT, Plan & Execute (**WIP**), LLMCompiler (**WIP**), ...

## Getting started

### Install from package manager

WIP [#1](https://github.com/RobinQu/instinct.cpp/issues/1)

### Build from sources

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


### Quick start

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


### What's next

You can learn more about this frameworks by following links below:

* [Chaining](docs/chaining.md)
* [Built-in components](docs/components.md) 
* Read more about single-binary services
  * [doc-agent](./modules/instinct-examples/doc-agent/README.md) : Chat with your docs locally with privacy.
  * mini-assistant-api.


## Roadmap

Complete project plan is tracked at [Github Project](https://github.com/users/RobinQu/projects/1/views/1?layout=board).

| Milestone                                                     | Features                                                                                               | DDL  |
|---------------------------------------------------------------|--------------------------------------------------------------------------------------------------------|------|
| v0.1.0                                                        | Long-short memory, PDF/TXT/DOCX ingestor, `Chain` programing paradigm, RAG reference app `doc-agent`   | 3.29 |
| [v0.1.1](https://github.com/RobinQu/instinct.cpp/milestone/1) | Performance tuning, RAG evaluation,  Function calling agent                                            | 4.16 |
| [v0.1.2](https://github.com/RobinQu/instinct.cpp/milestone/2) | OpenAI Assistant API initial implementation, single-binary reference app `mini-assistant`              | 4.30 |
| v0.1.3                                                        | Benchmarks, packages.                                                                                  | 5.17 |
| v0.2.0                                                        | Documentations. Features will be frozen.                                                               | 5.31 |


Contributions are welcomed! You can join [discord server](https://discord.gg/5cVnVyh3), or contact me via [email](mailto:robinqu@gmail.com).