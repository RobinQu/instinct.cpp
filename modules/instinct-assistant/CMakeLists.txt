cmake_minimum_required(VERSION 3.26)
project(instinct-assistant VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(LIBRARY_TARGET_NAME assistant)

SET(${LIBRARY_TARGET_NAME}_HDR
        include/instinct/assistant_global.hpp
        include/instinct/assistant_test_global.hpp
        include/instinct/assistant/v2/service/assistant_facade.hpp
        include/instinct/assistant/v2/service/assistant_service.hpp
        include/instinct/assistant/v2/service/file_service.hpp
        include/instinct/assistant/v2/service/run_service.hpp
        include/instinct/assistant/v2/service/thread_service.hpp
        include/instinct/assistant/v2/service/vector_store_service.hpp
        include/instinct/assistant/v2/endpoint/assistant_controller.hpp
        include/instinct/assistant/v2/endpoint/thread_controller.hpp
        include/instinct/assistant/v2/endpoint/run_controller.hpp
        include/instinct/assistant/v2/endpoint/run_step_controller.hpp
        include/instinct/assistant/v2/endpoint/vector_store_controller.hpp
        include/instinct/assistant/v2/endpoint/vector_store_file_controller.hpp
        include/instinct/assistant/v2/endpoint/vector_store_file_batch_controller.hpp
        include/instinct/assistant/v2/endpoint/file_controller.hpp
        include/instinct/assistant/v2/endpoint/base_controller.hpp
        include/instinct/assistant/v2/service/message_service.hpp
        include/instinct/assistant/v2/service/impl/assistant_service_impl.hpp
        include/instinct/assistant/v2/service/impl/thread_service_impl.hpp
        include/instinct/assistant/v2/tool/validation_utils.hpp
        include/instinct/assistant/v2/tool/entity_sql_utils.hpp
        include/instinct/assistant/v2/service/impl/message_service_impl.hpp
        include/instinct/assistant/v2/service/impl/file_service_impl.hpp
        include/instinct/assistant/v2/service/impl/run_service_impl.hpp
        include/instinct/assistant/v2/task_handler/run_object_task_handler.hpp
        include/instinct/assistant/v2/endpoint/message_controller.hpp
        include/instinct/assistant/v2/tool/assistant_api_application_context_factory.hpp
        include/instinct/assistant/v2/tool/request_utils.hpp
        include/instinct/assistant/v2/task_handler/file_object_task_handler.hpp
        include/instinct/assistant/v2/data_mapper/vector_store_data_mapper.hpp
        include/instinct/assistant/v2/data_mapper/vector_store_file_data_mapper.hpp
        include/instinct/assistant/v2/service/impl/vector_store_service_impl.hpp
        include/instinct/assistant/v2/tool/simple_retriever_operator.hpp
        include/instinct/assistant/v2/data_mapper/vector_store_file_batch_data_mapper.hpp
        include/instinct/assistant/v2/toolkit/summary_guided_file_search.hpp
        include/instinct/assistant/v2/task_handler/file_batch_object_background_task.hpp
        include/instinct/assistant/v2/db/db_migration.hpp
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
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)

install(DIRECTORY include/
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/instinct"
        COMPONENT instinct_agent
)

add_subdirectory(assets)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()


message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")
