cmake_minimum_required(VERSION 3.26)
project(instinct_core)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})
set(${LIBRARY_TARGET_NAME}_HDR
        include/instinct/tools/StringUtils.hpp
        include/instinct/tools/http/HttpUtils.hpp
        include/instinct/tools/HttpRestClient.hpp
        include/instinct/CoreGlobals.hpp
        include/instinct/tools/http/HttpClientException.hpp
        include/instinct/functional/IRunnable.hpp
        include/instinct/tools/TensorUtils.hpp
        include/instinct/tools/FunctionUtils.hpp
        include/instinct/tools/Assertions.hpp
        include/instinct/tools/ChronoUtils.hpp
        include/instinct/functional/ReactiveFunctions.hpp
        include/instinct/tools/CodecUtils.hpp
        include/instinct/tools/http/IHttpClient.hpp
        include/instinct/tools/http/CURLHttpClient.hpp
        include/instinct/functional/StepFunctions.hpp
        include/instinct/functional/IContext.hpp
        include/instinct/functional/JSONContextPolicy.hpp
        include/instinct/functional/RunnableChain.hpp
        include/instinct/functional/BaseRunnable.hpp
        include/instinct/tools/ProtobufUtils.hpp
        include/instinct/functional/IConfigurable.hpp
        include/instinct/functional/Xn.hpp
        include/instinct/CoreTestGlobals.hpp
        include/instinct/tools/DocumentUtils.hpp
        include/instinct/tools/MetadataSchemaBuilder.hpp
        include/instinct/tools/SystemUtils.hpp
        include/instinct/tools/file_vault/IFileVault.hpp
        include/instinct/tools/file_vault/FileSystemFileVault.hpp
        include/instinct/tools/file_vault/IFileVaultResourceProvider.hpp
        include/instinct/tools/file_vault/HttpURLResourceProvider.hpp
        include/instinct/tools/file_vault/BaseFileVaultResourceProvider.hpp
        include/instinct/tools/HashUtils.hpp
        include/instinct/tools/IOUtils.hpp
        include/instinct/tools/SnowflakeIDGenerator.hpp
        include/instinct/tools/file_vault/TempFile.hpp
        include/instinct/tools/RandomUtils.hpp
        include/instinct/exception/InstinctException.hpp
        include/instinct/exception/ClientException.hpp
        include/instinct/ioc/ManagedApplicationContext.hpp
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

message(STATUS "CURL_INCLUDE_DIR: " ${CURL_INCLUDE_DIR})
message(STATUS "ICU_INCLUDE_DIR: " ${ICU_INCLUDE_DIR})
message(STATUS "ICU_LIBRARIES: " ${ICU_LIBRARIES})
target_include_directories(${LIBRARY_TARGET_NAME} INTERFACE ${ICU_INCLUDE_DIR})


target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE
        instinct::proto
        bshoshany-thread-pool::bshoshany-thread-pool
        crossguid::crossguid
        RPP::rpp
        fmt::fmt
        nlohmann_json::nlohmann_json
        ${ICU_LIBRARIES}
        aklomp::base64
        tsl::ordered_map
        fmtlog::fmtlog
        CURL::libcurl
        uriparser::uriparser
        hash-library::hash-library
        pantor::inja
        cpptrace::cpptrace
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


