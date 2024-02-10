# langchain.cpp

LLM magics for cpp world.

# Features

## Chaining Expression

```c++

auto chain = (
    langchian::core::make_context("topic", langchain::core::RunnablePassthrough())
    | prompt
    | model
    | output_parser  
);
chain.invoke("ice cream");

for(const auto& chunk in chain.stream("ice stream")) {
    std::cout << chunk << std::endl;
}

chain.batch(["ice stream", "noodles"])
```

# Roadmap

## v0.1 - Model I/O

* [ ] PromptTemplate
* [ ] OllamaLLM
  * generate/chat/embedding
  * stream

## v0.2 - Model I/O

* [ ] LCEL Composition: Pipeline & parallelism
* [ ] LLMs: OpenAI API, llama.cpp

## v0.3 - Retriever

* [ ] faiss vectorstore
* [ ] document store
* [ ] Retriever classes
* [ ] official package release

## v1.0 - with ChamberAI app


## v1.1 - Agents framework



# References


* API Endpoints
  * [ollama](https://github.com/jmorganca/ollama/blob/main/docs/api.md)
  * [OpenAI](https://platform.openai.com/docs/api-reference)
* 