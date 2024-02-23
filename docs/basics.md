# Components

## instinct-core

* Model I/O
  * Input: PromptValue and PromptTemplates
  * Language models: LLMModel, ChatLLMModel, Embedding
  * Output: StringOutputParser, JSONOutputParser, ...
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


## instinct-rag

RAG related utilities, including
* Memory: vector indexers, topK retrievers
* Built-in chains:
  * RAGChain
  * ...


## instinct-cli

TODO

## instinct-serve

TODO