# doc-agent

A tiny yet powerful agent as CLI application. Help text from `--help-all` is self-explanatory. 

```shell
% doc-agent --help-all
🤖 DocAgent: Chat with your documents locally with privacy.
Usage: /Users/robinqu/Workspace/github/robinqu/instinct.cpp/build/Debug/modules/instinct-apps/doc-agent/doc-agent [OPTIONS] SUBCOMMAND

Options:
  -h,--help                   Print this help message and exit
  --help-all                  Expand all help
  --db_path TEXT REQUIRED     DB file path for botch vector store and doc store.
  -v,--verbose                A flag to enable verbose log
[Option Group: 🧠 Provider for chat model]
  Model provider for chat model
  Options:
    --chat_model_provider ENUM:value in {jina_ai->5,llama_cpp->4,llm_studio->3,local->2,ollama->1,openai->0} OR {5,4,3,2,1,0}
                                Specify chat model to use for chat completion.
    --chat_model_api_key TEXT   API key for commercial services like OpenAI. Leave blank for services without ACL.
    --chat_model_endpoint TEXT  Endpoint for chat model API.
    --chat_model_model_name TEXT
                                Specify name of the model to be used.
[Option Group: 🧠 Provider for embedding model]
  Ollama, OpenAI API, or any OpenAI API compatible servers are supported.
  Options:
    --embedding_model_provider ENUM:value in {jina_ai->5,llama_cpp->4,llm_studio->3,local->2,ollama->1,openai->0} OR {5,4,3,2,1,0}
                                Specify embedding model to use.
    --embedding_model_api_key TEXT
                                API key for commercial services like OpenAI. Leave blank for services without ACL.
    --embedding_model_endpoint TEXT
                                Endpoint for text embedding model, .e.g. 'https://api.openai.com/v1/api/embeddings' for OpenAI.
    --embedding_model_model_name TEXT
                                Specify name of the model to be used.
[Option Group: 🧠 Provider for reranker model]
  Currently only Jina.ai and local model are supported.
  Options:
    --reranker_model_provider ENUM:value in {jina_ai->5,llama_cpp->4,llm_studio->3,local->2,ollama->1,openai->0} OR {5,4,3,2,1,0}
                                Specify reranker model provider to use.
    --reranker_model_api_key TEXT
                                API key for commercial services like Jina.ai. Leave blank for services without ACL.
    --reranker_model_endpoint TEXT
                                Endpoint for reranker model API.
    --reranker_model_model_name TEXT
                                Specify name of the model to be used.
[Option Group: 🔍 Retriever]
  Options for building retriever
  Options:
    --retriever_version INT [2]
                                Version 1: Use ChunkedMultiVectorRetriever only.
                                Version 2: Use ChunkedMultiVectorRetriever, and BM25 keyword-based Retriever together with a local reranker.

  [Option Group: Options for ChunkedMultiVectorRetriever]
    Options:
      --child_chunk_size INT:INT in [200 - 10000] [200]
                                  chunk size for child document
      --parent_chunk_size INT:INT in [0 - 1000000] [0]
                                  chunk size for parent document. Zero means disabling parent document splitter.
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
  💼 Analyze a single document and build database of learned context data. Proper values should be offered for Embedding model, Chat model, DocStore, VecStore and Retriever mentioned above.
  Options:
    --force                     A flag to force rebuild of database, which means existing db files will be deleted. Use this option with caution!
  [Option Group: Data source]
    Options:
      -f,--file TEXT REQUIRED     Path to the document you want analyze
      -t,--type TEXT:{PDF,DOCX,MD,TXT,PARQUET} [TXT]
                                  File format of assigned document. Supported types are PDF,TXT,MD,DOCX,PARQUET
      --parquet_mapping TEXT      Mapping format for parquet columns. e.g. 1:t,2:m:parent_doc_id:int64,3:m:source:varchar.
      --source_limit UINT [0]     Limit max entries from data source. It's supported only part of ingestors including PARQUET. Zero means no limit.

serve
  💃 Start a OpenAI API compatible server with database of learned context. Proper values should be offered for Chat model, DocStore, VecStore and Retriever mentioned above.
  Options:
    -p,--port INT [9090]        Port number which API server will listen
```

Following line is example command to embed a local PDF and start up server with local Ollama service as model provider.  

```shell
doc-agent --verbose \
    --retriever_version=1 \
    --child_chunk_size=200 \
    --chat_model_provider=ollama \
    --chat_model_model_name=mistral:latest \
    --chat_model_endpoint=http://192.168.0.132/api/embeddings \
    --embedding_model_provider=ollama \
    --embedding_model_model_name=all-minilm:latest \
    --embedding_model_endpoint=http://192.168.0.132/api/chat \
    --db_path=/tmp/rag_eval_v1.db \
    --vector_table_dimension=384 \
build \
  --force \
  --file=attention_is_all_you_need.pdf \
  --type=PARQUET \
  --parquet_mapping=0:txt,1:metadata:source:varchar \
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
