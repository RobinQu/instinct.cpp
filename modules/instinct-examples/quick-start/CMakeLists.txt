project(quick-start)

cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 20)

add_executable(quick_start quick_start.cpp)


target_link_libraries(quick_start instinct::retrieval)