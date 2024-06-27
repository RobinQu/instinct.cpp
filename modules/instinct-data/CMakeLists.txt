cmake_minimum_required(VERSION 3.26)
project(instinct_data)

set(LIBRARY_TARGET_NAME data)
set(${LIBRARY_TARGET_NAME}_HDR
        include/instinct/data_global.hpp
        include/instinct/database/data_template.hpp
        include/instinct/database/duckdb/duckdb_data_template.hpp
        include/instinct/database/connection_pool.hpp
        include/instinct/database/connection_pool.hpp
        include/instinct/database/managed_connection.hpp
        include/instinct/database/duckdb/duckdb_connection_pool.hpp
        include/instinct/database/db_utils.hpp
        include/instinct/database/duckdb/duckdb_connection.hpp
        include/instinct/database/database_migration.hpp
        include/instinct/object_store/object_store.hpp
        include/instinct/object_store/file_system_object_store.hpp
        include/instinct/task_scheduler/task_scheduler.hpp
        include/instinct/task_scheduler/in_process_task_queue.hpp
        include/instinct/task_scheduler/base_task_scheduler.hpp
        include/instinct/task_scheduler/thread_pool_task_scheduler.hpp
)

add_library(
        ${LIBRARY_TARGET_NAME} INTERFACE
        ${${LIBRARY_TARGET_NAME}_HDR}
)
add_library(instinct::data
        ALIAS ${LIBRARY_TARGET_NAME})

target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE
        instinct::core
        instinct::proto
        duckdb::duckdb
        concurrentqueue::concurrentqueue
)

target_include_directories(${LIBRARY_TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)


if(BUILD_TESTING)
    add_subdirectory(test)
endif()


message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")


