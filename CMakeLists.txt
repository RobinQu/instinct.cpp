cmake_minimum_required(VERSION 3.27)
project(langchain_cpp)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(thirdparty/fmt)
find_package(fmt)



add_library(langchain_core modules/langchain-core/src/prompts/BasePromptTemplate.cpp
        modules/langchain-core/src/prompts/BasePromptTemplate.h
        modules/langchain-core/src/prompts/PromptValue.cpp
        modules/langchain-core/src/prompts/PromptValue.h
        modules/langchain-core/src/prompts/StringPromptTemplate.cpp
        modules/langchain-core/src/prompts/StringPromptTemplate.h
        modules/langchain-core/src/documents/Document.cpp
        modules/langchain-core/src/documents/Document.h
        modules/langchain-core/src/messages/BaseMessage.cpp
        modules/langchain-core/src/messages/BaseMessage.h
        modules/langchain-core/src/prompts/StringPromptValue.cpp
        modules/langchain-core/src/prompts/StringPromptValue.h
        modules/langchain-core/src/messages/ChatMessage.cpp
        modules/langchain-core/src/messages/ChatMessage.h
        modules/langchain-core/src/messages/HumanMessage.cpp
        modules/langchain-core/src/messages/HumanMessage.h
        modules/langchain-core/src/messages/FunctionMessage.cpp
        modules/langchain-core/src/messages/FunctionMessage.h
        modules/langchain-core/src/messages/SystemMessage.cpp
        modules/langchain-core/src/messages/SystemMessage.h
        modules/langchain-core/src/prompts/ChatPromptValue.cpp
        modules/langchain-core/src/prompts/ChatPromptValue.h
        modules/langchain-core/src/prompts/ChatPromptTemplate.cpp
        modules/langchain-core/src/prompts/ChatPromptTemplate.h
        modules/langchain-core/src/prompts/FewShotPromptTemplate.cpp
        modules/langchain-core/src/prompts/FewShotPromptTemplate.h
        modules/langchain-core/src/prompts/PipelineTemplate.cpp
        modules/langchain-core/src/prompts/PipelineTemplate.h)
target_link_libraries(langchain_core fmt::fmt)



add_executable(langchain_cpp main.cpp)

target_link_libraries(langchain_cpp langchain_core)
