# doc-agent

A tiny yet powerful agent as CLI application. Help text from `--help-all` is self-explanatory. 

```shell
% doc-agent --help-all
🤖 DocAgent: Chat with your documents locally with privacy.
Usage: doc-agent [OPTIONS] SUBCOMMAND

Options:
  -h,--help                   Print this help message and exit
  --help-all                  Expand all help
  --db_path TEXT REQUIRED     DB file path for botch vetcor store and doc store.
  -v,--verbose                A flag to enable verbose log
[Option Group: 🧠 Provider for embedding model]
  Ollama, OpenAI API, or any OpenAI API compatible servers are supported. Defaults to a local running Ollama service using llama2:latest model.
  Options:
    --chat_model_provider TEXT:{ollama,openai} [ollama]
                                Specify chat model to use for chat completion.
    --chat_model_api_key TEXT   API key for comercial services like OpenAI. Leave blank for services without ACL.
    --chat_model_host TEXT [localhost]
                                Host name for API endpoint, .e.g. 'api.openai.com' for OpenAI.
    --chat_model_port INT [11434]
                                Port number for API service.
    --chat_model_protocol ENUM:value in {http->1,https->2} OR {1,2} [1]
                                HTTP protocol for API service.
    --chat_model_model_name TEXT [llama2:latest]
                                Specify name of the model to be used.
[Option Group: 🧠 Provider for chat model]
  Ollama, OpenAI API, or any OpenAI API compatible servers are supported. Defaults to a local running Ollama service using llama2:latest model.
  Options:
    --embedding_model_provider TEXT:{ollama,openai} [ollama]
                                Specify embedding model to use.
    --embedding_model_api_key TEXT
                                API key for comercial services like OpenAI. Leave blank for services without ACL.
    --embedding_model_host TEXT [localhost]
                                Host name for API endpoint, .e.g. 'api.openai.com' for OpenAI.
    --embedding_model_port INT [11434]
                                Port number for API service.
    --embedding_model_protocol ENUM:value in {http->1,https->2} OR {1,2} [1]
                                HTTP protocol for API service.
    --embedding_model_model_name TEXT [llama2:latest]
                                Specify name of the model to be used.
[Option Group: 🔍 Retriever]
  Options for building retriever
  [Option Group: base_retriever]
    A must-to-have base retriever that handles original documents.
    [Exactly 1 of the following options is required]
    Options:
      --plain_vector_retriever    Enable VectorStoreRetiever.
      --parent_child_retriever    Enable ChunkedMultiVectorRetriever.
      --summary_guided_retriever  Enable MultiVectorGuidance with summary guidance.
      --hypothetical_quries_guided_retriever
                                  Enable MultiVectorGuidance with hypothetical queries.
  [Option Group: Options for ChunkedMultiVectorRetriever]
    Options:
      --child_chunk_size INT:INT in [200 - 10000] [200]
                                  chunk size for child document
      --parent_chunk_size INT:INT in [0 - 1000000] [0]
                                  chunk size for parent document. Zero means disabling parent document splitter.
  [Option Group: query_rewriter]
    Adaptor retrievers that rewrites original query.
    [At most 1 of the following options are allowed]
    Options:
      --multi_query_retriever     Enable MultiQueryRetriever.
[Option Group: 🔢 VectorStore]
  Options:
    --vector_table_dimension UINT:INT bounded to [1 - 8192] REQUIRED
                                Dimension of embedding vector.
    --vector_table_name TEXT [embedding_table]
                                Table name for embedding table.
[Option Group: 📖 DocStore]
  Options:
    --doc_table_name TEXT [doc_table]
                                Table name for documents

Subcommands:
build
  💼 Anaylize a single document and build database of learned context data. Proper values should be offered for Embedding model, Chat model, DocStore, VecStore and Retriever mentioned above.
  Options:
    --force                     A flag to force rebuild of database, which means existing db files will be deleted. Use this option with caution!
  [Option Group: Data source]
    Options:
      -f,--file TEXT REQUIRED     Path to the document you want analyze
      -t,--type TEXT:{PDF,DOCX,MD,TXT,PARQUET} [TXT]
                                  File format of assigned document. Supported types are PDF,TXT,MD,DOCX,PARQUET
      --parquet_mapping TEXT      Mapping format for parquet columns. e.g. 1:t,2:m:parent_doc_id:int64,3:m:source:varchar.

serve
  💃 Start a OpenAI API compatible server with database of learned context. Proper values should be offered for Chat model, DocStore, VecStore and Retriever mentioned above.
  Options:
    -p,--port INT [9090]        Port number which API server will listen
```

Example command to embed a local PDF and start up OpenAI-like chat completion API server at `localhost:9090`.  

```shell
doc-agent --verbose \
  --parent_child_retriever \
  --chat_model_model_name=starling-lm:latest \
  --embedding_model_model_name=all-minilm:latest \
  --db_path=/tmp/doc_agent.db \
  --vector_table_dimension=384 \
  build \
  --force \
  --file=attention_is_all_you_need.pdf \
  --type=PDF \
  serve \
  --port=9090
```

After few seconds, the server should be ready. Feel free to test with you question about documents.

```shell
curl --location --request POST 'http://localhost:9090/v1/chat/completions' \
--header 'Content-Type: application/json' \
--data-raw '{
    "messages": {
        "role" : "user",
        "content": "Please explain what is multi-head attention. "
    },
    "stream": false
}'
```
