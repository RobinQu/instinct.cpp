cmake_minimum_required(VERSION 3.26)
project(instinct_data)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})
set(${LIBRARY_TARGET_NAME}_HDR
        include/DataGlobals.hpp
        include/database/IDataMapper.hpp
        include/database/duckdb/DuckDBDataMapper.hpp
        include/database/IConnectionPool.hpp
        include/database/BaseConnectionPool.hpp
        include/database/ManagedConnection.hpp
        include/database/duckdb/DuckDBConnectionPool.hpp
        include/database/DBUtils.hpp
        include/database/duckdb/DuckDBConnection.hpp
        include/database/DatabaseMigration.hpp
        include/database/IDatabaseMigration.hpp
        include/database/duckdb/TransactionAware.hpp
)



add_library(
        ${LIBRARY_TARGET_NAME} INTERFACE
        ${${LIBRARY_TARGET_NAME}_HDR}
)
add_library(instinct::data
        ALIAS ${LIBRARY_TARGET_NAME})


find_package(inja REQUIRED)
find_package(duckdb REQUIRED)

target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE
        instinct::core
        instinct::proto
        pantor::inja
        duckdb::duckdb
)


target_include_directories(${LIBRARY_TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS ${LIBRARY_TARGET_NAME}
        EXPORT  ${PROJECT_NAME}
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/instinct" COMPONENT dev)

install(DIRECTORY include/
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/instinct"
        COMPONENT instinct_core
)

#install(
#        EXPORT InstinctTargets
#        NAMESPACE INSTINCT::
#        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake"
#        COMPONENT instinct-core
#)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()


message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")


