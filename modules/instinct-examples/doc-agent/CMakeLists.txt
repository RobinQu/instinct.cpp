cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 20)
project(doc-agent)

add_executable(doc-agent src/doc-agent.cpp)

FetchContent_Declare(
        CLI11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.4.2
        FIND_PACKAGE_ARGS # will invoke find_package first
)
FetchContent_MakeAvailable(CLI11)

target_link_libraries(doc-agent instinct::retrieval instinct::server CLI11::CLI11)