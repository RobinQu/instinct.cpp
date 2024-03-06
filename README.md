# instinct.cpp

`instinct.cpp` is a framework for developing AI Agent applications powered by language model.

## Features

* Privacy first LLM providers: Built on top of `llama.cpp`, wrapped in interface in Modern C++ flavor.
* Building blocks for common application patterns like RAG.
* Programing models: Extensive `Chain` and `Flowgraph`.
* Embeddable: header only and multi-platform compatability.

## Getting started

### Installation

For macs with Apple silicons at this moment:

```shell
brew install instinct-all
```

### Build from sources

System Requirements:

* OS: Debian X64, Linux X64, macOS arm64
* Preinstalled system libraries:
  * icu 72+
  * protobuf
  * openssl

```shell
git submodule update --init --recursive

```


## Roadmap

| Milestone | Features                                                                   | DDL  |
|-----------|----------------------------------------------------------------------------|------|
| M1        | Ollama API,llama.cpp, Chain basics                                         | 3.8  |
| M2        | Long-short memory, PDF/TXT/HTML parsers, RAG reference app, v0.1.0 release | 3.29 |
| M3        | Agent flowgraph, ChatAgent with function calling, v0.2.0 release           | 4.26 |
| M4        | linux/macOS CI build brew package, deb/yum package                         | 5.10 |

## Documentations

* Programing Models
  * Chain
  * Flowgraph
* Reference apps
  * RAG
  * ChatAgent
* Components
  * LLM Provider
  * Memory
  * Function calling toolkit