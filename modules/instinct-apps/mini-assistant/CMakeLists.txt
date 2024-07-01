project(mini-assistant)

if (NOT TARGET instinct::assistant)
    message(WARNING "Target instinct::assistant is not found. will skip mini-assistant module")
    return()
endif ()

cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 20)

add_executable(mini-assistant src/mini-assistant.cpp)
target_link_libraries(mini-assistant PUBLIC instinct::retrieval instinct::assistant CLI11::CLI11)
target_link_libraries(mini-assistant PRIVATE instinct::assistant::resources)
