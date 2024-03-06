cmake_minimum_required(VERSION 3.27)
project(instinct-proto)

set(CMAKE_CXX_STANDARD 20)

#INCLUDE(FindProtobuf)
find_package(Protobuf CONFIG REQUIRED)

add_library(proto-objects ${CMAKE_CURRENT_LIST_DIR}/src/retrieval.proto)

add_library(instinct::proto
        ALIAS proto-objects)

target_link_libraries(proto-objects PUBLIC protobuf::libprotobuf)

set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

target_include_directories(proto-objects PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")

protobuf_generate(
        TARGET proto-objects
        IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}/src"
        PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
)
