ARG UBUNTU_VERSION=23.04

FROM robinqu/instinct-builder-base as builder

WORKDIR /src
COPY . /src
RUN --mount=type=cache,target=/root/.conan2/p,sharing=private \
    conan install . --build=missing --output-folder=build
RUN --mount=type=cache,target=/root/.conan2/p,sharing=private \
     mkdir build && \
    cd build && \
    cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release && \
    cmake --build .



FROM ubuntu:$UBUNTU_VERSION

WORKDIR /app
COPY --from=builder /src/build/modules/instinct-examples/mini-assitant/mini-assistant /app/
ENTRYPOINT /app/mini-assistant

