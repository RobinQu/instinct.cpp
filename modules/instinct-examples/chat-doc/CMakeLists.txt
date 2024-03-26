cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 20)
project(chat-doc)

add_executable(chat-doc src/chat-doc.cpp)

find_package(CLI11 REQUIRED)

target_link_libraries(chat-doc instinct::retrieval instinct::server CLI11::CLI11)