# Docker build recipes

## devcontainer image

```shell
docker buildx build --push --platform=linux/amd64,linux/arm64 -t robinqu/instinct-cpp-devcontainer:latest -f ./dockerfile/devcontainer.dockerfile .
```

## Builder image

**All commands should be run at project root.**

To build base image:

```shell
docker buildx build --push --platform=linux/amd64,linux/arm64 -t robinqu/instinct-builder-base:latest -f ./dockerfile/builder-base.dockerfile . 
```

To run bash in builder base image:

```shell
docker run --pull=always -it --rm robinqu/instinct-builder-base:latest
```

## Runtime image for `mini-assistant` 

To build mini-assistant image:

```shell
docker buildx build --push --platform=linux/amd64,linux/arm64 -t robinqu/instinct-mini-assistant:latest -f ./dockerfile/mini-assistant.dockerfile . 
```

To run `mini-assistant`:

```shell
docker run \
  -v /tmp/mini-assistant:/data \
  -p 9091:9091 \
  robinqu/instinct-mini-assistant:latest \
  --file_store_path=/tmp/mini-assistant-files/ \
  --db_file_path=/tmp/mini-assistant.db -p 9091
```
