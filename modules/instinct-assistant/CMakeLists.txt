cmake_minimum_required(VERSION 3.26)
project(instinct-assistant VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})

SET(${LIBRARY_TARGET_NAME}_HDR
        include/instinct/AssistantGlobals.hpp
        include/instinct/AssistantTestGlobals.hpp
        include/instinct/assistant/v2/service/AssistantFacade.hpp
        include/instinct/assistant/v2/service/IAssistantService.hpp
        include/instinct/assistant/v2/service/IFileService.hpp
        include/instinct/assistant/v2/service/IRunService.hpp
        include/instinct/assistant/v2/service/IThreadService.hpp
        include/instinct/assistant/v2/service/IVectorStoreService.hpp

        include/instinct/assistant/v2/endpoint/AssistantController.hpp
        include/instinct/assistant/v2/endpoint/ThreadController.hpp
        include/instinct/assistant/v2/endpoint/RunController.hpp
        include/instinct/assistant/v2/endpoint/RunStepController.hpp
        include/instinct/assistant/v2/endpoint/VectorStoreController.hpp
        include/instinct/assistant/v2/endpoint/VectorStoreFileController.hpp
        include/instinct/assistant/v2/endpoint/VectorStoreFileBatchController.hpp
        include/instinct/assistant/v2/endpoint/FileController.hpp
        include/instinct/assistant/v2/endpoint/BaseController.hpp
        include/instinct/assistant/v2/service/IMessageService.hpp
        include/instinct/assistant/v2/service/impl/AssistantServiceImpl.hpp
        include/instinct/assistant/v2/service/impl/ThreadServiceImpl.hpp
        include/instinct/assistant/v2/tool/ValidationUtils.hpp
        include/instinct/assistant/v2/tool/EntitySQLUtils.hpp
        include/instinct/assistant/v2/service/impl/MessageServiceImpl.hpp
        include/instinct/assistant/v2/service/impl/FileServiceImpl.hpp
        include/instinct/assistant/v2/service/impl/RunServiceImpl.hpp
        include/instinct/assistant/v2/task_handler/RunObjectTaskHandler.hpp
        include/instinct/assistant/v2/endpoint/MessageController.hpp
        include/instinct/assistant/v2/tool/AssistantAPIApplicationContextFactory.hpp
        include/instinct/assistant/v2/tool/RequestUtils.hpp
        include/instinct/assistant/v2/task_handler/FileObjectTaskHandler.hpp
        include/instinct/assistant/v2/data_mapper/VectorStoreDataMapper.hpp
        include/instinct/assistant/v2/data_mapper/VectorStoreFileDataMapper.hpp
        include/instinct/assistant/v2/service/impl/VectorStoreServiceImpl.hpp
        include/instinct/assistant/v2/tool/SimpleRetrieverOperator.hpp
        include/instinct/assistant/v2/data_mapper/VectorStoreFileBatchDataMapper.hpp
        include/instinct/assistant/v2/toolkit/SummaryGuidedFileSearch.hpp
        include/instinct/assistant/v2/task_handler/FileBatchObjectBackgroundTask.hpp
        include/instinct/assistant/v2/db/DBMigration.hpp
)


add_library(
        ${LIBRARY_TARGET_NAME} INTERFACE
        ${${LIBRARY_TARGET_NAME}_HDR}
)



add_library(instinct::agent
        ALIAS ${LIBRARY_TARGET_NAME})

target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE instinct::llm instinct::proto instinct::server instinct::data instinct::retrieval)

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
