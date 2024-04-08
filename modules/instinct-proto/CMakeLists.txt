cmake_minimum_required(VERSION 3.26)
project(instinct-proto)

set(CMAKE_CXX_STANDARD 20)


find_package(Protobuf CONFIG REQUIRED)

add_library(proto-objects STATIC
        src/core.proto
        src/retrieval.proto
        src/llm.proto
        src/agent.proto
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
