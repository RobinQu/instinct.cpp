# mini-assistants

## Quick start

## Kickoff server

### Option 1: Using prebuilt binary 

Download binary package from [release page](https://github.com/RobinQu/instinct.cpp/releases) and unpack. 

```shell
./mini-assistant \
  --file_store_path=/tmp/mini-assistant-files/  \
  --db_file_path=/tmp/mini-assistant.db \
  -p 9091
```

### Option 2: Using Docker

```shell
docker run robinqu/mini-assistant:latest \
  -v /tmp/mini-assistant:/data \
  -p 9091:9091
  --file_store_path=/tmp/mini-assistant-files/ \
  --db_file_path=/tmp/mini-assistant.db -p 9091
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
Usage: mini-assistant [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  --help-all                  Expand all help
  -p,--port INT [9091]        Port number which API server will listen
  --db_file_path TEXT REQUIRED
                              Path for DuckDB database file.
  --file_store_path TEXT:DIR REQUIRED
                              Path for root directory of local object store.
  --model_provider TEXT:{ollama,openai} [openai]
                              Specify chat model to use for chat completion.
  -v,--verbose                A flag to enable verbose log
[Option Group: 🧠 OpenAI configuration]
  OpenAI API, or any OpenAI API compatible servers are supported. Defaults to OpenAI public server.
  Options:
    --openai_api_key TEXT       API key for commercial services like OpenAI. Leave blank for services without ACL. API key is also retrieved from env variable named OPENAI_API_KEY.
    --openai_host TEXT [api.openai.com]
                                Host name for API endpoint, .e.g. 'api.openai.com' for OpenAI.
    --openai_port INT [443]     Port number for API service.
    --openai_protocol ENUM:value in {http->1,https->2} OR {1,2} [2]
                                HTTP protocol for API service.
    --openai_model_name TEXT [gpt-3.5-turbo]
                                Specify name of the model to be used.
[Option Group: 🧠 Ollama configuration]
  Ollama, OpenAI API, or any OpenAI API compatible servers are supported. Defaults to a local running Ollama service using llama2:latest model.
  Options:
    --ollama_host TEXT [localhost]
                                Host name for Ollama API endpoint.
    --ollama_port INT [11434]   Port number for Ollama API endpoint.
    --chat_model_protocol ENUM:value in {http->1,https->2} OR {1,2} [1]
                                HTTP protocol for Ollama API endpoint.
    --chat_model_model_name TEXT [llama2:latest]
                                Specify name of the model to be used.
```


## Implementation details

* A thread pool based task scheduler is used to handle jobs for `run` objects.
* DuckDB is used for convention structured data as well as vector data. Many improvements can be done. [More details](https://github.com/users/RobinQu/projects/1/views/1?pane=issue&itemId=62004973). 
* More technical details about Assistant API can be found in [docs/assistant_api.md](../../../docs/assistant_api.md).
* known issues:
  * Function calling requires OpenAI's `gpt-3.5` or `gpt-4` series. Function calling with opensourced LLMs is possible, and it's on top of my TODO list.
  * All timestamps are currently printed in microsecond precision while it's printed in epoch seconds in official APIs.
  * Only function tool is supported. `file-search` is next to come. `code-interpreter` is scheduled as later.