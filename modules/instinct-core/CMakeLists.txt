cmake_minimum_required(VERSION 3.26)
project(instinct_core)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})
set(${LIBRARY_TARGET_NAME}_HDR
        include/tools/StringUtils.hpp
        include/tools/http/HttpUtils.hpp
        include/tools/HttpRestClient.hpp
        include/CoreGlobals.hpp
        include/tools/http/HttpClientException.hpp
        include/functional/IRunnable.hpp
        include/tools/TensorUtils.hpp
        include/tools/FunctionUtils.hpp
        include/tools/Assertions.hpp
        include/tools/ChronoUtils.hpp
        include/functional/ReactiveFunctions.hpp
        include/tools/CodecUtils.hpp
        include/tools/http/IHttpClient.hpp
        include/tools/http/CURLHttpClient.hpp
        include/functional/StepFunctions.hpp
        include/functional/IContext.hpp
        include/functional/JSONContextPolicy.hpp
        include/functional/RunnableChain.hpp
        include/functional/BaseRunnable.hpp
        include/tools/ProtobufUtils.hpp
        include/functional/IConfigurable.hpp
        include/functional/Xn.hpp
        include/CoreTestGlobals.hpp
        include/tools/DocumentUtils.hpp
        include/tools/MetadataSchemaBuilder.hpp
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
        crossguid::crossguid
        instinct::proto
        RPP::rpp
        fmt::fmt
        nlohmann_json::nlohmann_json
        icu::icu
        aklomp::base64
        tsl::ordered_map
        fmtlog::fmtlog
        crossguid::crossguid
        CURL::libcurl
        uriparser::uriparser
)

#set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES
#        VERSION ${${PROJECT_NAME}_VERSION}
#        PUBLIC_HEADER "${${LIBRARY_TARGET_NAME}_HDR}"
#)

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


