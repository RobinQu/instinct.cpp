cmake_minimum_required(VERSION 3.26)
project(instinct_data)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})
set(${LIBRARY_TARGET_NAME}_HDR
        include/instinct/DataGlobals.hpp
        include/instinct/database/IDataTemplate.hpp
        include/instinct/database/duckdb/DuckDBDataTemplate.hpp
        include/instinct/database/IConnectionPool.hpp
        include/instinct/database/BaseConnectionPool.hpp
        include/instinct/database/ManagedConnection.hpp
        include/instinct/database/duckdb/DuckDBConnectionPool.hpp
        include/instinct/database/DBUtils.hpp
        include/instinct/database/duckdb/DuckDBConnection.hpp
        include/instinct/database/DatabaseMigration.hpp
        include/instinct/database/IDatabaseMigration.hpp
        include/instinct/database/duckdb/TransactionAware.hpp
        include/instinct/object_store/IObjectStore.hpp
        include/instinct/object_store/FileSystemObjectStore.hpp
        include/instinct/task_scheduler/ITaskScheduler.hpp
        include/instinct/task_scheduler/InProcessTaskQueue.hpp
        include/instinct/task_scheduler/BaseTaskScheduler.hpp
        include/instinct/task_scheduler/ThreadPoolTaskScheduler.hpp
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


