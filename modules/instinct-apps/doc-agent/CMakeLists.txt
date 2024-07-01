cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 20)
project(doc-agent)

if (NOT (WITH_DUCKDB AND WITH_DUCKX AND WITH_PDFIUM))
    message(WARNING "duckdb, duckx and pdfium are required for doc-agent.")
    return()
endif ()

add_executable(doc-agent src/doc-agent.cpp)
target_link_libraries(doc-agent instinct::retrieval instinct::server CLI11::CLI11)
