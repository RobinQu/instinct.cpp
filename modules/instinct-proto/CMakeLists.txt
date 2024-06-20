cmake_minimum_required(VERSION 3.26)
project(instinct-proto)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
SET(protobuf_BUILD_TESTS OFF)
SET(ABSL_PROPAGATE_CXX_STD ON)
FetchContent_Declare(
        Protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
        GIT_TAG v27.1
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS # will invoke find_package first
)
FetchContent_MakeAvailable(Protobuf)
message(STATUS "proto"  ${Protobuf_INCLUDE_DIRS})
#include_directories(${Protobuf_INCLUDE_DIRS})
#include_directories(${CMAKE_CURRENT_BINARY_DIR})
#find_package(Protobuf CONFIG REQUIRED)

add_library(proto-objects STATIC
        src/core.proto
        src/retrieval.proto
        src/llm.proto
        src/agent.proto
        src/assistant_api_v2.proto
        src/embedding_api.proto
        src/chat_completion_api.proto
        src/ollama_api.proto
        src/data.proto
)

add_library(instinct::proto
        ALIAS proto-objects)

target_link_libraries(proto-objects PUBLIC
        protobuf::libprotoc
        protobuf::libprotobuf
)

set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

target_include_directories(proto-objects PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")



protobuf_generate(
        TARGET proto-objects
        IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}/src"
        PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
)


install(TARGETS proto-objects
        EXPORT  ${PROJECT_NAME}
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/instinct" COMPONENT dev)
