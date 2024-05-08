ARG UBUNTU_VERSION=23.04

FROM robinqu/instinct-builder-base as builder

ARG TARGETPLATFORM

ARG TARGETOS
ARG TARGETARCH
ARG TARGETVARIANT

WORKDIR /src
COPY . /src
# copy conan profile
COPY ./dockerfile/conan-profile/ /tmp/
RUN --mount=type=cache,id=id=conan-$TARGETARCH$TARGETVARIANT,target=/root/.conan2,sharing=locked \
    mkdir -p /root/.conan2/profiles/ && cp /tmp/$(uname -m)/* /root/.conan2/profiles/
RUN --mount=type=cache,id=id=conan-$TARGETARCH$TARGETVARIANT,target=/root/.conan2,sharing=locked \
    conan install . --build=missing --output-folder=build
RUN --mount=type=cache,id=id=conan-$TARGETARCH$TARGETVARIANT,target=/root/.conan2,sharing=locked \
    cd build && \
    cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release && \
    cmake --build .


FROM ubuntu:$UBUNTU_VERSION

WORKDIR /app
COPY --from=builder /src/build/modules/instinct-examples/mini-assistant/mini-assistant /app/
ENTRYPOINT ["/app/mini-assistant"]

