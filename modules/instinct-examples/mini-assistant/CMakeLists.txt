project(mini-assistant)

cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 20)


FetchContent_Declare(
        CLI11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.4.2
        FIND_PACKAGE_ARGS # will invoke find_package first
)
FetchContent_MakeAvailable(CLI11)

add_executable(mini-assistant src/mini-assistant.cpp)
target_link_libraries(mini-assistant PUBLIC instinct::retrieval instinct::agent CLI11::CLI11)

target_link_libraries(mini-assistant PRIVATE instinct::assistant::resources)
