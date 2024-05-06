cmake_minimum_required(VERSION 3.26)
project(instinct-assistant VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})

SET(${LIBRARY_TARGET_NAME}_HDR
        include/AssistantGlobals.hpp
        include/AssistantTestGlobals.hpp
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
        include/assistant/v2/service/impl/ThreadServiceImpl.hpp
        include/assistant/v2/tool/ValidationUtils.hpp
        include/assistant/v2/tool/EntitySQLUtils.hpp
        include/assistant/v2/service/impl/MessageServiceImpl.hpp
        include/assistant/v2/service/impl/FileServiceImpl.hpp
        include/assistant/v2/service/impl/RunServiceImpl.hpp
        include/assistant/v2/task_handler/OpenAIToolAgentRunObjectTaskHandler.hpp
        include/assistant/v2/endpoint/MessageController.hpp
        include/assistant/v2/tool/IApplicationContextFactory.hpp
        include/assistant/v2/tool/RequestUtils.hpp
)


add_library(
        ${LIBRARY_TARGET_NAME} INTERFACE
        ${${LIBRARY_TARGET_NAME}_HDR}
)



add_library(instinct::agent
        ALIAS ${LIBRARY_TARGET_NAME})

target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE instinct::llm instinct::proto instinct::server instinct::data)

target_include_directories(${LIBRARY_TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}/instinct>)

install(DIRECTORY include/
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/instinct"
        COMPONENT instinct_agent
)

add_subdirectory(assets)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()


message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")
