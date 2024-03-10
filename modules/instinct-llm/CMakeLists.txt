cmake_minimum_required(VERSION 3.27)
project(instinct_llm)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})

set(${LIBRARY_TARGET_NAME}_HDR  src/All.hpp
        src/LLMGlobals.hpp
        src/chain/IChain.hpp
        src/chain/LLMChain.hpp
        src/llm/BaseLLM.hpp
        src/chat_model/BaseChatModel.hpp
        src/model/ILanguageModel.hpp
        src/commons/OllamaCommons.hpp
        src/llm/OllamaLLM.hpp
        src/chat_model/OllamaChat.hpp
        src/embedding_model/OllamaEmbedding.hpp
        src/memory/IChatMemory.hpp
        src/prompt/IPromptTemplate.hpp
        src/prompt/IChatPromptTemplate.hpp
        src/prompt/StringPromptTemplate.hpp
        src/prompt/PlainPromptTemplate.hpp
        src/prompt/PlainChatTemplate.hpp
        src/prompt/FewShotPromptTemplate.hpp
        src/prompt/IExampleSelector.hpp
        src/prompt/MutableExampleSelector.hpp
        src/prompt/PassthroughExampleSelector.hpp
        src/prompt/LengthBasedExampleSelector.h
        src/output_parser/ProtobufMessageOutputParser.hpp
        src/output_parser/StringOutputParser.hpp
        src/prompt/MessageUtils.hpp
        src/prompt/ChatPromptBuilder.hpp
        src/tokenizer/Tokenizer.hpp
        src/tokenizer/RegexTokenizer.hpp
        src/tokenizer/TiktokenTokenizer.hpp
        src/tokenizer/BPETokenRanksReader.hpp
        src/tokenizer/GPT2BPEFileReader.hpp
        src/tokenizer/TiktokenBPEFileReader.hpp
)

#set(${LIBRARY_TARGET_NAME}_SRC
#
#)

add_library(
        ${LIBRARY_TARGET_NAME} INTERFACE
#        ${${LIBRARY_TARGET_NAME}_SRC}
        ${${LIBRARY_TARGET_NAME}_HDR}
)


add_library(instinct::llm
        ALIAS ${LIBRARY_TARGET_NAME})


#set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES
#        #        VERSION ${${PROJECT_NAME}_VERSION}
#        PUBLIC_HEADER "${${LIBRARY_TARGET_NAME}_HDR}"
#)


target_include_directories(${LIBRARY_TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE instinct::core)

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