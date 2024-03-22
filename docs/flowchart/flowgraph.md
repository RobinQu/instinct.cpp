# flowgraph


## Dynamic API 

```c++

using ic::core::flowgraph;
using ic::core::factory;


void chain1() {
    auto g = create_graph(); // FlowGraphPtr
    auto prompt = create_prompt("tell me more about {input}."); // PromptTemplatePtr
    auto model = create_model({"model", "llama2"}); // BaseLLMsPtr
    auto parser = create_output_parser(); // OutputParserPtr
    connect(g, prompt, model);
    connect(g, model, parser);
    
    auto executor = create_task_flow_executor(); // FlowGraphExecutorPtr
    auto ctx = create_sync_context("input", "ice cream"); // SyncContext
    auto result = invoke(g, executor, ctx); // std::string
}


void chain2() {
    auto prompt = create_prompt("tell me more about {input}."); // PromptTemplatePtr
    auto model = create_model({"model", "llama2"}); // BaseLLMsPtr
    auto parser = create_output_parser(); // OutputParserPtr
    
    
    auto g = create_graph() << prompt << model << parser; // FlowGraphAdaptor
}

void chain3() {

    auto prompt = create_prompt("tell me more about {input}."); // PromptTemplatePtr
    auto model = create_model({"model", "llama2"}); // BaseLLMsPtr
    auto parser = create_output_parser(); // OutputParserPtr
    auto vector_index = create_faiss_index(); // FaissIndexPtr
    auto retriver = create_topk_retriever(); // TopKRetrieverPtr
    
    auto g = create_graph();
    
    g.Connect(retriver, prompt)
    g.Connect(prompt, model, parser)
    
    
    
}
```


## CLI calling


```shell
# ad-hoc run for pipeline in `./context_dir`
instict-cli -c ./context_dir -e '{"input": "ice cream"}'"


# setup http service at port 8888
instinct-cli -c ./context_dir -d http -p 8888
```



```
./context_dir
--- index.yaml
--- db
```

index.yaml

```yaml
- name: simple rag chain
  components: 
    - name: prompt1
      type: StringPromptTemplate
      arguments:
        - content: Tell me more about {input}
    - name: model
      type: OllamaChat
      arguments: 
        - name: model
          value: llama2
    - name: output_parser
      type: GenerationOutputParser
  connections: 
    - [prompt1, model]
    - [model, output_parser]
```



