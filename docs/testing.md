# Testing 

In order to make all components working, some env variables are needed to correctly configured.

```shell
export OPENAI_CHAT_API_ENDPOINT=<API ENDPOINT FOR CHAT MODEL>
export OPENAI_EMBEDDING_API_ENDPOINT=<API ENDPOINT FOR EMBEDDING MODEL>
export OPENAI_API_KEY=<YOUR OPENAI API KEY>
export OPENAI_CHAT_MODEL=<CHAT MODEL NAME>
export OPENAI_EMBEDDING_MODEL=<EMBEDDING MODEL NAME>
export OPENAI_EMBEDDING_DIM=<EMBEDDING MODEL DIMENSION>
export SERP_APIKEY=<SERP API KEY>
export JINA_API_KEY=<JINA API KEY>
```

Unit tests are registered to ctest, so you can trigger test runner by running following command in `build` folder:

```shell
ctest
```

