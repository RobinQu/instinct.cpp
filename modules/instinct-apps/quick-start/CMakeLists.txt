cmake_minimum_required(VERSION 3.26)
project(quick-start)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

#FetchContent_Declare(
#        instinct.cpp
#        GIT_REPOSITORY git@github.com:RobinQu/instinct.cpp.git
#        GIT_TAG v0.1.5
#        GIT_SHALLOW 1
#)
#FetchContent_MakeAvailable(instinct.cpp)

add_subdirectory(../../../ ${CMAKE_BINARY_DIR}/instinct.cpp)

add_executable(quick_start quick_start.cpp)
target_link_libraries(quick_start INTERFACE instinct::llm)