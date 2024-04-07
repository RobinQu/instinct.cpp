cmake_minimum_required(VERSION 3.26)
project(instinct-agent VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)


set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})

SET(${LIBRARY_TARGET_NAME}_HDR
        include/AgentGlobals.hpp


)


add_library(
        ${LIBRARY_TARGET_NAME} INTERFACE
        ${${LIBRARY_TARGET_NAME}_HDR}
)

add_library(instinct::agent
        ALIAS ${LIBRARY_TARGET_NAME})


target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE instinct::llm)

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
