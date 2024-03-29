# Components

## instinct-core

* Model I/O
  * Input: PromptValue and PromptTemplates
  * Language models: LLMModel, ChatLLMModel, Embedding
  * Output: GenerationOutputParser, JSONOutputParser, ...
* Programing paradigms
  * Chains: Pre-defined chains for widely-used cases. Chaining in free-form style, which could be found in many Python library is not available due to natures of C++. See discussions in `chain.md`. 
  * Flowgraph: Dynamic graph-based scheduler 

## instinct-llm

### LLM providers

|                       | Completion | Chat | Embedding |
|-----------------------|------------|------|-----------|
| Ollama                | Y          | Y    | Y         |
| OpenAI (or alike) API | Y          | Y    | Y         |
| llama.cpp             | Y          | Y    | Y         |

### Built-in chains

* LLMChain
* ...


## instinct-retrievers

RAG related utilities, including

* Document manipulation:
  * DocumentSource
  * DocumentTransformers
  * TextSplitter
* Memory:
  * Document store
  * Top-k Retriever
* Built-in chains:
  * RAGChain
  * ...


## instinct-server

## Composable Chain Server

Process model: 

```
- DaemonProcess
  |- LLM Provider 1
  |- LLM Provider 2
  |- ...
  |- HTTP Server for instinct objects, like `Chain`s. 
```

* LLM Provider: inference process, e.g. nitro server.
