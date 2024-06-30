# mini-assistants

Please read [Limitations of mini-assistant](https://github.com/users/RobinQu/projects/1/views/1?pane=issue&itemId=67421127) before continue.

## Quick start

## Kickoff server

### Option 1: Using prebuilt binary 

Download binary package from [release page](https://github.com/RobinQu/instinct.cpp/releases) and unpack. 

```shell
./mini-assistant \
  --file_store_path=/tmp/mini-assistant-files/  \
  --db_file_path=/tmp/mini-assistant.db \
  -p 9091 \
  --verbose
```

### Option 2: Using Docker

```shell
docker run --rm \
  -v /tmp/mini-assistant-db:/data/db \
  -v /tmp/mini-assistant-files:/data/files \
  -p 9091:9091 \
  -e OPENAI_API_KEY=${OPENAI_API_KEY} \
  robinqu/instinct-mini-assistant:latest \
  --file_store_path=/data/files \
  --db_file_path=/data/db/mini-assistant.db \
  -p 9091 \
  --verbose
```


## Working with `OpenAI SDK`

```python
from openai import OpenAI

# IMPORTANT: set `base_url` to accessible endpoint for mini-assistant 
client = OpenAI(
        base_url="http://localhost:9091/v1"
    )

assistant = client.beta.assistants.create(
    name="Math Tutor",
    instructions="You are a personal math tutor. Answer questions briefly, in a sentence or less.",
    model="gpt-3.5-turbo",
)
```


## CLI Usage

```text
🐬 mini-assistant - Local Assistant API at your service
Usage: /Users/robinqu/Workspace/github/robinqu/instinct.cpp/build/Debug/modules/instinct-apps/mini-assistant/mini-assistant [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  --help-all                  Expand all help
  -p,--port INT [9091]        Port number which API server will listen
  --db_file_path TEXT REQUIRED
                              Path for DuckDB database file.
  --file_store_path TEXT REQUIRED
                              Path for root directory of local object store. Will be created if it doesn't exist yet.
  --agent_executor_type TEXT:{llm_compiler,openai_tool} [llm_compiler]
                              Specify agent executor type. `llm_compiler` enables parallel function calling with opensourced models like mistral series and llama series, while `openai_tool` relies on official OpenAI function calling capability to direct agent workflow.
  -v,--verbose                A flag to enable verbose log
[Option Group: chat_model]
  Configuration for chat model
  Options:
    --chat_model_provider ENUM:value in {jina_ai->5,llama_cpp->4,llm_studio->3,local->2,ollama->1,openai->0} OR {5,4,3,2,1,0} [0]
                                Specify chat model to use for chat completion.
    --chat_model_name TEXT      Specify chat model to use for chat completion. Default to gpt-3.5-turbo for OpenAI, llama3:8b for Ollama. Note that some model providers will ignore the passed model name and use the model currently loaded instead.
    --chat_model_api_key TEXT   API key for commercial services like OpenAI. Leave blank for services without ACL. API key is also retrieved from env variable named OPENAI_API_KEY.
    --chat_model_endpoint TEXT  Endpoint for chat model API, .e.g. 'https://api.openai.com/v1/chat_completion' for OpenAI.
[Option Group: embedding_model]
  Configuration for embedding model
  Options:
    --embedding_model_provider ENUM:value in {jina_ai->5,llama_cpp->4,llm_studio->3,local->2,ollama->1,openai->0} OR {5,4,3,2,1,0} [0]
                                Specify model to use for embedding.
    --embedding_model_name TEXT Specify model to use for embedding . Default to text-embedding-3-small for OpenAI, all-minilm:latest for Ollama. Note that some model providers will ignore the passed model name and use the model currently loaded instead.
    --embedding_model_dim INT:POSITIVE
                                Dimension of given embedding model.
    --embedding_model_api_key TEXT
                                API key for commercial services like OpenAI. Leave blank for services without ACL. API key is also retrieved from env variable named OPENAI_API_KEY.
    --embedding_model_endpoint TEXT
                                Endpoint for text embedding model API.
[Option Group: ranking_model]
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
[Option Group: Options for LLMCompilerAgentExecutor]
  Options for LLMCompiler-based agent executor
  Options:
    --max_replan INT [6]        Max count for replan
```


To run assistant api service with chat model served by  `llama.cpp` server and text embedding model served by `Ollama`:

```
mini-assistant \ 
  --db_file_path /tmp/assistant_api.db \
  --file_store_path /tmp/mini-assistant-files \
  --agent_executor_type=llm_compiler \
  --chat_model_provider=llama_cpp \
  --chat_model_endpoint=http://192.168.0.134:8000/v1/chat/completions \
  --embedding_model_provider=ollama \
  --embedding_model_endpoint=http://192.168.0.134:31434/v1/embeddings \
  --embedding_model_name=all-minilm:latest \
  --verbose
```

