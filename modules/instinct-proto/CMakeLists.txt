cmake_minimum_required(VERSION 3.26)
project(instinct-proto)

set(CMAKE_CXX_STANDARD 20)

add_library(proto-objects
        core.proto
        retrieval.proto
        llm.proto
        agent.proto
        assistant_api_v2.proto
        embedding_api.proto
        chat_completion_api.proto
        ollama_api.proto
        data.proto
)

add_library(instinct::proto ALIAS proto-objects)

target_link_libraries(proto-objects
    protobuf::protobuf
)

set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

if (protobuf_FOUND)
    message(STATUS "protobuf_INCLUDE_DIRS: " ${protobuf_INCLUDE_DIRS})
    message(STATUS "Protobuf_PROTOC_EXECUTABLE" ${Protobuf_PROTOC_EXECUTABLE})
endif ()

target_include_directories(proto-objects PUBLIC
    "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>"
)

protobuf_generate(
        TARGET proto-objects
        IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}"
        PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
)

