# Instinct Chain


`Chain`s are reusable components that can be:

1. used with unified  `Runnable` interface, which consists of `Invoke`, `Batch` , `Stream` functions.
2. served with gRPC server through `RunnableServiceStub`.
3. working with repl tools through `instinct-cli`



```yaml

name: my_chat_chain
type: VanillaRAGChain
components:
  - name: retriever
    type: DuckDBVectorRetriever
    properties:
      db_path: "xxxx"
  - name: prompt_template_1
    type: StringPromptTemplate
    properties:
      template: Tell me about today's weather.
  - name: few_shot_template_1
    type: FewShotPromptTemplate
    properties:
      prefix: "examples are:"
      suffix: "please xxx"
  - name: ollama_model
    type: OllamaChat
  - name: parser
    type: StringOutputParser
```