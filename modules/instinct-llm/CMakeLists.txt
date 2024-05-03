cmake_minimum_required(VERSION 3.26)
project(instinct_llm)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})

set(${LIBRARY_TARGET_NAME}_HDR
        include/LLMGlobals.hpp
        include/chain/LLMChain.hpp
        include/llm/BaseLLM.hpp
        include/chat_model/BaseChatModel.hpp
        include/model/ILanguageModel.hpp
        include/commons/OllamaCommons.hpp
        include/llm/OllamaLLM.hpp
        include/chat_model/OllamaChat.hpp
        include/embedding_model/OllamaEmbedding.hpp
        include/memory/IChatMemory.hpp
        include/prompt/IPromptTemplate.hpp
        include/prompt/IChatPromptTemplate.hpp
        include/prompt/StringPromptTemplate.hpp
        include/prompt/PlainPromptTemplate.hpp
        include/prompt/PlainChatPromptTemplate.hpp
        include/prompt/FewShotPromptTemplate.hpp
        include/prompt/IExampleSelector.hpp
        include/prompt/MutableExampleSelector.hpp
        include/prompt/PassthroughExampleSelector.hpp
        include/prompt/LengthBasedExampleSelector.h
        include/output_parser/IOutputParser.hpp
        include/output_parser/StructuredGenerationOutputParser.hpp
        include/output_parser/BaseOutputParser.hpp
        include/prompt/MessageUtils.hpp
        include/tokenizer/Tokenizer.hpp
        include/tokenizer/RegexTokenizer.hpp
        include/tokenizer/TiktokenTokenizer.hpp
        include/tokenizer/BPETokenRanksReader.hpp
        include/tokenizer/GPT2BPEFileReader.hpp
        include/tokenizer/TiktokenBPEFileReader.hpp
        include/memory/EphemeralChatMemory.hpp
        include/output_parser/BaseOutputParser.hpp
        include/memory/BaseChatMemory.hpp
        include/chat_model/OpenAIChat.hpp
        include/commons/OpenAICommons.hpp
        include/embedding_model/OpenAIEmbedding.hpp
        include/LLMTestGlobals.hpp
        include/llm/OpenAILLM.hpp
        include/prompt/BasePromptTemplate.hpp
        include/prompt/BaseChatPromptTemplate.hpp
        include/input_parser/IInputParser.hpp
        include/input_parser/BaseInputParser.hpp
        include/chain/MessageChain.hpp
        include/output_parser/ProtobufMessageOutputParser.hpp
        include/output_parser/MultilineGenerationOutputParser.hpp
        include/input_parser/PromptValueVariantInputParser.hpp
        include/output_parser/StringOutputParser.hpp

        include/toolkit/IFunctionTool.hpp
        include/toolkit/BaseFunctionTool.hpp
        include/toolkit/LambdaFunctionTool.hpp
        include/toolkit/IFunctionToolkit.hpp
        include/toolkit/BaseFunctionToolkit.hpp
        include/toolkit/LocalToolkit.hpp
        include/agent/patterns/react/ReACTAgentThoughtOutputParser.hpp
        include/agent/patterns/react/ReACTAgentStateInputParser.hpp
        include/agent/patterns/react/Agent.hpp
        include/agent/executor/IAgentExecutor.hpp
        include/agent/executor/BaseAgentExecutor.hpp
        include/agent/BaseWorker.hpp
        include/agent/BaseSolver.hpp
        include/toolkit/ProtoMessageFunctionTool.hpp
        include/toolkit/BaseSearchTool.hpp
        include/toolkit/builtin/SerpAPI.hpp
        include/toolkit/builtin/LLMMath.hpp
        include/agent/patterns/openai_tool/Agent.hpp
        include/agent/state/IStateManager.hpp
        include/agent/state/InMemoryStateManager.hpp
)

find_package(exprtk REQUIRED)

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
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE instinct::core exprtk::exprtk)

install(TARGETS ${LIBRARY_TARGET_NAME}
        EXPORT  ${PROJECT_NAME}
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/instinct" COMPONENT dev)

install(DIRECTORY include/
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/instinct"
        COMPONENT instinct_llm
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")