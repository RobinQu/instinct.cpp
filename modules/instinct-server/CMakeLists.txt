cmake_minimum_required(VERSION 3.26)
project(instinct-server)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})

set(${LIBRARY_TARGET_NAME}_HDR
        include/ServerGlobals.hpp
        include/http/IChainServer.hpp
        include/http/PlainChainServer.hpp
)

add_library(
        ${LIBRARY_TARGET_NAME} INTERFACE
        #        ${${LIBRARY_TARGET_NAME}_SRC}
        ${${LIBRARY_TARGET_NAME}_HDR}
)

add_library(instinct::server
        ALIAS ${LIBRARY_TARGET_NAME})

target_include_directories(${LIBRARY_TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE
        instinct::retrieval
        httplib::httplib
)

install(TARGETS ${LIBRARY_TARGET_NAME}
        EXPORT  ${PROJECT_NAME}
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_TARGET_NAME}" COMPONENT dev)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")