# mini-assistants

## Quick start with docker

```shell

```

## CLI Usage

```text
🐬 mini-assistant - Local Assistant API at your service
Usage: mini-assistant [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  --help-all                  Expand all help
  -p,--port INT [9091]        Port number which API server will listen
  --db_file_path TEXT         Path for DuckDB database file.
  --file_store_path TEXT      Path for root directory of local object store.
  -v,--verbose                A flag to enable verbose log
```


## Implementation details

* A thread pool based task scheduler is used to handle jobs for `run` objects.
* DuckDB is used for convention structured data as well as vector data. Many improvements can be done here: [Item#62004973](https://github.com/users/RobinQu/projects/1/views/1?pane=issue&itemId=62004973). 
* More technical details can be found in [docs/assistant_api.md](../../../docs/assistant_api.md).

