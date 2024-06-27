cmake_minimum_required(VERSION 3.26)
project(instinct-server)

set(LIBRARY_TARGET_NAME server)

set(${LIBRARY_TARGET_NAME}_HDR
        include/instinct/server_global.hpp
        include/instinct/server/managed_server.hpp
        include/instinct/endpoint/chain/multi_chain_controller.hpp
        include/instinct/server/httplib/http_lib_server.hpp
        include/instinct/endpoint/chat_completion/chat_completion_controller.hpp
        include/instinct/endpoint/chat_completion/chat_completion_request_input_parser.hpp
        include/instinct/endpoint/chat_completion/chat_completion_response_output_parser.hpp
        include/instinct/server/mountable.hpp
        include/instinct/server/server_lifecycle_handler.hpp
        include/instinct/server/httplib/http_lib_server_lifecycle_manager.hpp
        include/instinct/server/HttpController.hpp
        include/instinct/server/http_server_exception.hpp
        include/instinct/server/httplib/http_lib_session.hpp
        include/instinct/server/httplib/default_error_controller.hpp
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
        instinct::core
        httplib::httplib
)

install(TARGETS ${LIBRARY_TARGET_NAME}
        EXPORT  ${PROJECT_NAME}
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/instinct" COMPONENT dev)


install(DIRECTORY include/
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/instinct"
        COMPONENT instinct_server
)


if(BUILD_TESTING)
    add_subdirectory(test)
endif()

message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")