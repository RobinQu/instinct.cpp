# Docker build recipes

## Builder image

**All commands should be run at project root.**

To build base image:

```shell
docker build --push --platform=linux/amd64,linux/arm64/v8 -t robinqu/instinct-builder-base:latest -f ./dockerfile/builder-base.dockerfile . 
```

To run bash in builder base image:

```shell
docker run --pull=always -it --rm robinqu/instinct-builder-base:latest
```

## Runtime image for `mini-assistant` 

To build mini-assistant image:

```shell
docker build --push --platform=linux/amd64,linux/arm64/v8 -t robinqu/instinct-mini-assistant:latest -f ./dockerfile/mini-assistant.dockerfile . 
```

To run `mini-assistant`:

```shell
docker run robinqu/mini-assistant:latest \
  -v /tmp/mini-assistant:/data \
  -p 9091:9091
  --file_store_path=/tmp/mini-assistant-files/ \
  --db_file_path=/tmp/mini-assistant.db -p 9091
```