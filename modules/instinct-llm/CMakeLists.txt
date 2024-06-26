cmake_minimum_required(VERSION 3.26)
project(instinct_llm)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})

set(${LIBRARY_TARGET_NAME}_HDR
        include/instinct/LLMGlobals.hpp
        include/instinct/chain/LLMChain.hpp
        include/instinct/llm/BaseLLM.hpp
        include/instinct/chat_model/BaseChatModel.hpp
        include/instinct/model/ILanguageModel.hpp
        include/instinct/commons/OllamaCommons.hpp
        include/instinct/llm/OllamaLLM.hpp
        include/instinct/chat_model/OllamaChat.hpp
        include/instinct/embedding_model/OllamaEmbedding.hpp
        include/instinct/memory/IChatMemory.hpp
        include/instinct/prompt/IPromptTemplate.hpp
        include/instinct/prompt/IChatPromptTemplate.hpp
        include/instinct/prompt/StringPromptTemplate.hpp
        include/instinct/prompt/PlainPromptTemplate.hpp
        include/instinct/prompt/PlainChatPromptTemplate.hpp
        include/instinct/prompt/FewShotPromptTemplate.hpp
        include/instinct/prompt/IExampleSelector.hpp
        include/instinct/prompt/MutableExampleSelector.hpp
        include/instinct/prompt/PassthroughExampleSelector.hpp
        include/instinct/prompt/LengthBasedExampleSelector.h
        include/instinct/output_parser/IOutputParser.hpp
        include/instinct/output_parser/StructuredGenerationOutputParser.hpp
        include/instinct/output_parser/BaseOutputParser.hpp
        include/instinct/prompt/MessageUtils.hpp
        include/instinct/tokenizer/Tokenizer.hpp
        include/instinct/tokenizer/RegexTokenizer.hpp
        include/instinct/tokenizer/TiktokenTokenizer.hpp
        include/instinct/tokenizer/BPETokenRanksReader.hpp
        include/instinct/tokenizer/GPT2BPEFileReader.hpp
        include/instinct/tokenizer/TiktokenBPEFileReader.hpp
        include/instinct/memory/EphemeralChatMemory.hpp
        include/instinct/output_parser/BaseOutputParser.hpp
        include/instinct/memory/BaseChatMemory.hpp
        include/instinct/chat_model/OpenAIChat.hpp
        include/instinct/commons/OpenAICommons.hpp
        include/instinct/embedding_model/OpenAIEmbedding.hpp
        include/instinct/LLMTestGlobals.hpp
        include/instinct/llm/OpenAILLM.hpp
        include/instinct/prompt/BasePromptTemplate.hpp
        include/instinct/prompt/BaseChatPromptTemplate.hpp
        include/instinct/input_parser/IInputParser.hpp
        include/instinct/input_parser/BaseInputParser.hpp
        include/instinct/chain/MessageChain.hpp
        include/instinct/output_parser/ProtobufMessageOutputParser.hpp
        include/instinct/output_parser/MultilineGenerationOutputParser.hpp
        include/instinct/input_parser/PromptValueVariantInputParser.hpp
        include/instinct/output_parser/StringOutputParser.hpp

        include/instinct/toolkit/IFunctionTool.hpp
        include/instinct/toolkit/BaseFunctionTool.hpp
        include/instinct/toolkit/LambdaFunctionTool.hpp
        include/instinct/toolkit/IFunctionToolkit.hpp
        include/instinct/toolkit/BaseFunctionToolkit.hpp
        include/instinct/toolkit/LocalToolkit.hpp
        include/instinct/agent/patterns/react/ReACTAgentThoughtOutputParser.hpp
        include/instinct/agent/patterns/react/ReACTAgentStateInputParser.hpp
        include/instinct/agent/patterns/react/Agent.hpp
        include/instinct/agent/executor/IAgentExecutor.hpp
        include/instinct/agent/executor/BaseAgentExecutor.hpp
        include/instinct/agent/BaseWorker.hpp
        include/instinct/toolkit/ProtoMessageFunctionTool.hpp
        include/instinct/toolkit/BaseSearchTool.hpp
        include/instinct/toolkit/builtin/SerpAPI.hpp
        include/instinct/toolkit/builtin/LLMMath.hpp
        include/instinct/agent/patterns/openai_tool/OpenAIToolAgentExecutor.hpp
        include/instinct/agent/patterns/llm_compiler/LLMCompilerAgentExecutor.hpp
        include/instinct/agent/patterns/llm_compiler/LLMCompilerPlaner.hpp
        include/instinct/agent/patterns/llm_compiler/LLMCompilerJoiner.hpp
        include/instinct/agent/patterns/openai_tool/OpenAIToolAgentPlanner.hpp
        include/instinct/agent/patterns/llm_compiler/LLMCompilerPlanerAgentStateInputParser.hpp
        include/instinct/agent/patterns/llm_compiler/LLMCompilerPlanerThoughtOutputParser.hpp
        include/instinct/agent/patterns/llm_compiler/TaskGraphUtils.hpp
        include/instinct/agent/patterns/llm_compiler/LLMCompilerJoinerTaskGraphInputParser.hpp
        include/instinct/agent/patterns/llm_compiler/LLMCompilerJoinerResultOutputParser.hpp
        include/instinct/agent/LocalToolkitsWorker.hpp
        include/instinct/LLMObjectFactory.hpp
        include/instinct/ranker/BaseRankingModel.hpp
        include/instinct/model/IRankingModel.hpp
        include/instinct/ranker/LocalRankingModel.hpp
        include/instinct/embedding_model/LocalEmbeddingModel.hpp
)


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

target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE
        instinct::core
        instinct::transformer
        exprtk::exprtk
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")