cmake_minimum_required(VERSION 3.27)
project(instinct_core)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})
set(${LIBRARY_TARGET_NAME}_HDR
        src/ALL.hpp
        src/tools/SimpleHttpClient.hpp
        src/tools/HttpRequest.hpp
        src/tools/StringUtils.hpp
        src/tools/HttpUtils.hpp
        src/tools/HttpResponse.hpp
        src/tools/HttpRestClient.hpp
        src/CoreGlobals.hpp
        src/tools/HttpClientException.hpp
        src/functional/IRunnable.hpp
        src/tools/TensorUtils.hpp
        src/tools/FunctionUtils.hpp
        src/tools/Assertions.hpp
        src/tools/ChronoUtils.hpp
        src/functional/ReactiveFunctions.hpp
)
#set(${LIBRARY_TARGET_NAME}_SRC
#
#)

add_library(
        ${LIBRARY_TARGET_NAME} INTERFACE
#        ${${LIBRARY_TARGET_NAME}_SRC}
        ${${LIBRARY_TARGET_NAME}_HDR}
)

add_library(instinct::core
        ALIAS ${LIBRARY_TARGET_NAME})
target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE
        instinct::proto
        RPP::rpp
        fmt::fmt
        nlohmann_json::nlohmann_json
        httplib
        Taskflow
        ICU::uc
        ICU::i18n
        ICU::io
        base64
        tsl::ordered_map
        fmtlog
)

#set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES
##        VERSION ${${PROJECT_NAME}_VERSION}
#        PUBLIC_HEADER "${${LIBRARY_TARGET_NAME}_HDR}"
#)

target_include_directories(${LIBRARY_TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)

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


