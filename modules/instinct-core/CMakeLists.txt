cmake_minimum_required(VERSION 3.27)
project(langchain_core)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})
set(${LIBRARY_TARGET_NAME}_HDR
        src/ALL.hpp
        src/prompt/PromptTemplate.hpp
        src/prompt/PromptValue.hpp
        src/prompt/StringPromptTemplate.hpp
        src/document/Document.hpp
        src/message/Message.hpp
        src/prompt/StringPromptValue.hpp
        src/message/ChatMessage.hpp
        src/message/HumanMessage.hpp
        src/message/FunctionMessage.hpp
        src/message/SystemMessage.hpp
        src/prompt/ChatPromptValue.hpp
        src/prompt/ChatPromptTemplate.hpp
        src/prompt/FewShotPromptTemplate.hpp
        src/model/BaseLanguageModel.hpp
        src/model/LLMResult.hpp
        src/CoreTypes.hpp
        src/model/BaseEmbeddingModel.hpp
        src/model/BaseChatModel.hpp
        src/model/BaseLLM.hpp
        src/tools/SimpleHttpClient.hpp
        src/message/AIMessage.hpp
        src/tools/HttpRequest.hpp
        src/tools/StringUtils.hpp
        src/tools/HttpUtils.hpp
        src/tools/HttpResponse.hpp
        src/tools/HttpRestClient.hpp
        src/model/ChatResult.hpp
        src/model/ChatGeneration.hpp
        src/CoreGlobals.hpp
        src/model/Generation.hpp
        src/tools/HttpClientException.hpp
        src/prompt/PlainPromptTemplate.hpp
        src/prompt/MessagePromptTemplate.hpp
        src/prompt/AIMessagePromptTemplate.hpp
        src/prompt/ChatMessagePromptTemplate.hpp
        src/prompt/SystemMessagePromptTemplate.h
        src/prompt/HumanMessagePromptTemplate.hpp
        src/prompt/ExampleSelector.hpp
        src/prompt/LengthBasedExampleSelector.h
        src/prompt/MutableExampleSelector.hpp
        src/prompt/FewShotChatMessagePromptTemplate.hpp
        src/prompt/BaseChatPromptTemplate.hpp
        src/prompt/PassthroughExampleSelector.hpp
        src/chain/Chain.hpp
        src/prompt/Forwards.hpp
        src/prompt/BaseStringMessagePromptTemplate.hpp
        src/tools/TensorUtils.hpp
        src/tools/FunctionUtils.hpp
        src/tools/HttpChunkConnection.hpp
        src/tools/ResultIterator.hpp
        src/tools/HttpChunkResultIterator.hpp
        src/document/TextSplitter.hpp
        src/model/Embeddings.hpp
        src/document/RecursiveCharacterTextSplitter.hpp
)
#set(${LIBRARY_TARGET_NAME}_SRC
#
#)

add_library(
        ${LIBRARY_TARGET_NAME} INTERFACE
#        ${${LIBRARY_TARGET_NAME}_SRC}
        ${${LIBRARY_TARGET_NAME}_HDR}
)

add_library(langchain::core
        ALIAS ${LIBRARY_TARGET_NAME})
target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE
        fmt::fmt
        nlohmann_json::nlohmann_json
#        xtensor
        Boost::beast
        Boost::algorithm
        Boost::url
        Taskflow
#        RPP::rpp
        ICU::uc
        ICU::data
)

#set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES
##        VERSION ${${PROJECT_NAME}_VERSION}
#        PUBLIC_HEADER "${${LIBRARY_TARGET_NAME}_HDR}"
#)

target_include_directories(${LIBRARY_TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)

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


