project(mini-assistant)

cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 20)


find_package(CLI11 REQUIRED)
add_executable(mini-assistant src/mini-assistant.cpp)
target_link_libraries(mini-assistant PUBLIC instinct::retrieval instinct::agent CLI11::CLI11)

target_link_libraries(mini-assistant PRIVATE instinct::assistant::resources)
