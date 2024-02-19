cmake_minimum_required(VERSION 3.27)
project(langchain_core)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})


set(${LIBRARY_TARGET_NAME}_HDR src/prompt/BasePromptTemplate.h
        src/prompt/PromptValue.h
        src/prompt/StringPromptTemplate.h
        src/document/Document.h
        src/message/BaseMessage.h
        src/prompt/StringPromptValue.h
        src/message/ChatMessage.h
        src/message/HumanMessage.h
        src/message/FunctionMessage.h
        src/message/SystemMessage.h
        src/prompt/ChatPromptValue.h
        src/prompt/ChatPromptTemplate.h
        src/prompt/FewShotPromptTemplate.h
        src/prompt/PipelineTemplate.h
        src/model/BaseLanguageModel.h
        src/model/LLMResult.h
        src/CoreTypes.h
        src/document/Embedding.h
        src/model/BaseEmbeddingModel.h
        src/model/BaseChatModel.h
        src/Core.h
        src/message/MessageType.h
        src/model/BaseLLM.h
        src/CoreFunctions.h
)
set(${LIBRARY_TARGET_NAME}_SRC
        src/prompt/BasePromptTemplate.cpp
        src/prompt/PromptValue.cpp
        src/prompt/StringPromptTemplate.cpp
        src/document/Document.cpp
        src/message/BaseMessage.cpp
        src/prompt/StringPromptValue.cpp
        src/message/ChatMessage.cpp
        src/message/HumanMessage.cpp
        src/message/FunctionMessage.cpp
        src/message/SystemMessage.cpp
        src/prompt/ChatPromptValue.cpp
        src/prompt/ChatPromptTemplate.cpp
        src/prompt/FewShotPromptTemplate.cpp
        src/prompt/PipelineTemplate.cpp
        src/model/BaseLanguageModel.cpp
        src/model/LLMResult.cpp
        src/model/BaseChatModel.cpp
        src/model/BaseEmbeddingModel.cpp
        src/document/Embedding.cpp
        src/tools/SimpleHttpClient.cpp
        src/tools/SimpleHttpClient.h
        src/model/BaseLLM.cpp
        src/message/AIMessage.cpp
        src/message/AIMessage.h
        src/tools/HttpRequest.cpp
        src/tools/HttpRequest.h
        src/tools/StringUtils.h
        src/tools/HttpUtils.h
        src/tools/HttpResponse.cpp
        src/tools/HttpResponse.h
        src/tools/HttpRestClient.cpp
        src/tools/HttpRestClient.h
        src/tools/HttpChunkBodyIterator.cpp
        src/tools/HttpChunkBodyIterator.h
        src/model/ChatResult.cpp
        src/model/ChatResult.h
        src/model/ChatGeneration.cpp
        src/model/ChatGeneration.h
        src/CoreGlobals.h
        src/model/Generation.cpp
        src/model/Generation.h
        src/tools/HttpClientException.cpp
        src/tools/HttpClientException.h
        src/prompt/PromptTemplate.cpp
        src/prompt/PromptTemplate.h
        src/prompt/BaseMessagePromptTemplate.cpp
        src/prompt/BaseMessagePromptTemplate.h
        src/prompt/AIMessagePromptTemplate.cpp
        src/prompt/AIMessagePromptTemplate.h
        src/prompt/ChatMessagePromptTemplate.cpp
        src/prompt/ChatMessagePromptTemplate.h
        src/prompt/SystemMessagePromptTemplate.cpp
        src/prompt/SystemMessagePromptTemplate.h
        src/prompt/HumanMessagePromptTemplate.cpp
        src/prompt/HumanMessagePromptTemplate.h
        src/prompt/BaseExmapleSelector.cpp
        src/prompt/BaseExmapleSelector.h
        src/prompt/LengthBasedExampleSelector.cpp
        src/prompt/LengthBasedExampleSelector.h
        src/prompt/MutableExampleSelector.cpp
        src/prompt/MutableExampleSelector.h
        src/prompt/FewShotChatMessagePromptTemplate.cpp
        src/prompt/FewShotChatMessagePromptTemplate.h
        src/prompt/BaseChatPromptTemplate.cpp
        src/prompt/BaseChatPromptTemplate.h
        src/prompt/PassthroughExampleSelector.cpp
        src/prompt/PassthroughExampleSelector.h
        src/runnable/Runnable.cpp
        src/runnable/Runnable.h
        src/runnable/RunnableSequence.cpp
        src/runnable/RunnableSequence.h
        src/runnable/RunnableParallel.cpp
        src/runnable/RunnableParallel.h
        src/runnable/RunnableLambda.cpp
        src/runnable/RunnableLambda.h
        src/runnable/RunnableAssign.cpp
        src/runnable/RunnableAssign.h
        src/runnable/RunnablePick.cpp
        src/runnable/RunnablePick.h
        src/flowgraph/GraphNode.h
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
        Boost::beast
        Boost::algorithm
        Boost::url
        Taskflow
)

set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES
#        VERSION ${${PROJECT_NAME}_VERSION}
        PUBLIC_HEADER "${${LIBRARY_TARGET_NAME}_HDR}"
)

target_include_directories(${LIBRARY_TARGET_NAME}
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
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


