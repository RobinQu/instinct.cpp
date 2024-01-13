cmake_minimum_required(VERSION 3.27)
project(langchain_core)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})

set(${LIBRARY_TARGET_NAME}_HDR
        src/prompts/BasePromptTemplate.h
        src/prompts/PromptValue.h
        src/prompts/StringPromptTemplate.h
        src/documents/Document.h
        src/messages/BaseMessage.h
        src/prompts/StringPromptValue.h
        src/messages/ChatMessage.h
        src/messages/HumanMessage.h
        src/messages/FunctionMessage.h
        src/messages/SystemMessage.h
        src/prompts/ChatPromptValue.h
        src/prompts/ChatPromptTemplate.h
        src/prompts/FewShotPromptTemplate.h
        src/prompts/PipelineTemplate.h
        src/model/BaseLanguageModel.h
        src/model/LLMResult.h
        src/model/LLMGeneration.h
        src/types.h
        src/documents/Embedding.h
        src/model/BaseEmbeddingModel.h
        src/model/BaseChatModel.h
)
set(${LIBRARY_TARGET_NAME}_SRC
        src/prompts/BasePromptTemplate.cpp
        src/prompts/PromptValue.cpp
        src/prompts/StringPromptTemplate.cpp
        src/documents/Document.cpp
        src/messages/BaseMessage.cpp
        src/prompts/StringPromptValue.cpp
        src/messages/ChatMessage.cpp
        src/messages/HumanMessage.cpp
        src/messages/FunctionMessage.cpp
        src/messages/SystemMessage.cpp
        src/prompts/ChatPromptValue.cpp
        src/prompts/ChatPromptTemplate.cpp
        src/prompts/FewShotPromptTemplate.cpp
        src/prompts/PipelineTemplate.cpp
        src/model/BaseLanguageModel.cpp
        src/model/LLMResult.cpp
        src/model/LLMGeneration.cpp
        src/model/BaseChatModel.cpp
        src/model/BaseEmbeddingModel.cpp
        src/documents/Embedding.cpp

)

add_library(
        ${LIBRARY_TARGET_NAME}
        ${${LIBRARY_TARGET_NAME}_SRC}
        ${${LIBRARY_TARGET_NAME}_HDR}
)

add_library(langchain::core
        ALIAS ${LIBRARY_TARGET_NAME})
target_link_libraries(${LIBRARY_TARGET_NAME}
        fmt::fmt
        nlohmann_json::nlohmann_json
        xtensor
)

set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES
#        VERSION ${${PROJECT_NAME}_VERSION}
        PUBLIC_HEADER ${${LIBRARY_TARGET_NAME}_HDR}
)

target_include_directories(${LIBRARY_TARGET_NAME}
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>")

install(TARGETS ${LIBRARY_TARGET_NAME}
        EXPORT  ${PROJECT_NAME}
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_TARGET_NAME}" COMPONENT dev)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()


message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")


