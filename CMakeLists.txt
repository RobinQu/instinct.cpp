cmake_minimum_required(VERSION 3.27)
project(langchain_cpp)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(thirdparty/fmt)
find_package(fmt)



add_library(langchain_core modules/langchain-core/src/BasePromptTemplate.cpp
        modules/langchain-core/src/BasePromptTemplate.h
        modules/langchain-core/src/PromptValue.cpp
        modules/langchain-core/src/PromptValue.h
        modules/langchain-core/src/StringPromptTemplate.cpp
        modules/langchain-core/src/StringPromptTemplate.h)
target_link_libraries(langchain_core fmt::fmt)



add_executable(langchain_cpp main.cpp)

target_link_libraries(langchain_cpp langchain_core)
