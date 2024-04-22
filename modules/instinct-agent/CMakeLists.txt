cmake_minimum_required(VERSION 3.26)
project(instinct-agent VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})

SET(${LIBRARY_TARGET_NAME}_HDR
        include/AgentGlobals.hpp
        include/toolkit/IFunctionTool.hpp
        include/toolkit/BaseFunctionTool.hpp
        include/toolkit/LambdaFunctionTool.hpp
        include/toolkit/IFunctionToolkit.hpp
        include/toolkit/BaseFunctionToolkit.hpp
        include/toolkit/LocalToolkit.hpp
        include/agent/react/ReACTAgentThoughtOutputParser.hpp
        include/agent/react/ReACTAgentStateInputParser.hpp
        include/agent/react/Agent.hpp
        include/executor/IAgentExecutor.hpp
        include/executor/BaseAgentExecutor.hpp
        include/agent/BaseWorker.hpp
        include/agent/BaseSolver.hpp
        include/AgentTestGlobals.hpp
        include/toolkit/ProtoMessageFunctionTool.hpp
        include/toolkit/BaseSearchTool.hpp
        include/toolkit/builtin/SerpAPI.hpp
        include/toolkit/builtin/LLMMath.hpp
        include/assistant/v2/service/AssistantFacade.hpp
        include/assistant/v2/service/IAssistantService.hpp
        include/assistant/v2/service/IFileService.hpp
        include/assistant/v2/service/IRunService.hpp
        include/assistant/v2/service/IThreadService.hpp
        include/assistant/v2/service/IVectorStoreService.hpp

        include/assistant/v2/endpoint/AssistantController.hpp
        include/assistant/v2/endpoint/ThreadController.hpp
        include/assistant/v2/endpoint/RunController.hpp
        include/assistant/v2/endpoint/RunStepController.hpp
        include/assistant/v2/endpoint/VectorStoreController.hpp
        include/assistant/v2/endpoint/VectorStoreFileController.hpp
        include/assistant/v2/endpoint/VectorStoreFileBatchController.hpp
        include/assistant/v2/endpoint/FileController.hpp
        include/assistant/v2/endpoint/BaseController.hpp
        include/assistant/v2/service/IMessageService.hpp
        include/assistant/v2/service/impl/AssistantServiceImpl.hpp
)


add_library(
        ${LIBRARY_TARGET_NAME} INTERFACE
        ${${LIBRARY_TARGET_NAME}_HDR}
)

find_package(exprtk REQUIRED)

add_library(instinct::agent
        ALIAS ${LIBRARY_TARGET_NAME})

target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE instinct::llm instinct::proto exprtk::exprtk instinct::server)

target_include_directories(${LIBRARY_TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}/instinct>)

install(DIRECTORY include/
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/instinct"
        COMPONENT instinct_agent
)


if(BUILD_TESTING)
    add_subdirectory(test)
endif()


message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")
