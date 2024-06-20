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
        include/tools/SystemUtils.hpp
        include/tools/file_vault/IFileVault.hpp
        include/tools/file_vault/FileSystemFileVault.hpp
        include/tools/file_vault/IFileVaultResourceProvider.hpp
        include/tools/file_vault/HttpURLResourceProvider.hpp
        include/tools/file_vault/BaseFileVaultResourceProvider.hpp
        include/tools/HashUtils.hpp
        include/tools/IOUtils.hpp
        include/tools/SnowflakeIDGenerator.hpp
        include/tools/file_vault/TempFile.hpp
        include/tools/RandomUtils.hpp
        include/exception/InstinctException.hpp
        include/exception/ClientException.hpp
        include/ioc/ManagedApplicationContext.hpp
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

include(FetchContent)
FetchContent_Declare(
        RPP
        GIT_REPOSITORY https://github.com/victimsnino/ReactivePlusPlus.git
        GIT_TAG 103b882
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS NAMES RPP # will invoke find_package first
)
FetchContent_Declare(
        hash-library
        GIT_REPOSITORY https://github.com/stbrumme/hash-library.git
        GIT_TAG hash_library_v8
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS NAMES hash-library # will invoke find_package first
)
FetchContent_Declare(
        base64
        GIT_REPOSITORY https://github.com/aklomp/base64.git
        GIT_TAG v0.5.2
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS NAMES base64 # will invoke find_package first
)
FetchContent_Declare(
        bshoshany-thread-pool
        GIT_REPOSITORY https://github.com/bshoshany/thread-pool.git
        GIT_TAG v4.1.0
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS NAMES bshoshany-thread-pool # will invoke find_package first
)
FetchContent_Declare(
        cpptrace
        GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
        GIT_TAG v0.6.1
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS NAMES cpptrace # will invoke find_package first
)
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS NAMES nlohmann_json # will invoke find_package first
)
FetchContent_Declare(
        tsl-ordered-map
        GIT_REPOSITORY https://github.com/Tessil/ordered-map.git
        GIT_TAG v1.1.0
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS NAMES tsl-ordered-map # will invoke find_package first
)
#FetchContent_Declare(
#        fmt
#        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
#        GIT_TAG 10.2.1
#        GIT_SHALLOW 1
#        FIND_PACKAGE_ARGS NAMES fmt # will invoke find_package first
#)
FetchContent_Declare(
        fmtlog
        GIT_REPOSITORY https://github.com/MengRao/fmtlog.git
        GIT_TAG v2.2.1
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS NAMES fmtlog # will invoke find_package first
)
FetchContent_Declare(
        crossguid
        GIT_REPOSITORY https://github.com/graeme-hill/crossguid.git
        GIT_TAG v0.2.2
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS NAMES crossguid # will invoke find_package first
)
SET(URIPARSER_BUILD_TESTS OFF)
FetchContent_Declare(
        uriparser
        GIT_REPOSITORY https://github.com/uriparser/uriparser.git
        GIT_TAG uriparser-0.9.8
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS NAMES uriparser # will invoke find_package first
)
FetchContent_Declare(
        CURL
        GIT_REPOSITORY https://github.com/curl/curl.git
        GIT_TAG curl-8_8_0
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS NAMES CURL # will invoke find_package first
)
FetchContent_Declare(
        ICU
        GIT_REPOSITORY https://github.com/viaduck/icu-cmake.git
        GIT_TAG 480105d
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS NAMES ICU # will invoke find_package first
)
SET(BUILD_ICU ON)
SET(ICU_BUILD_VERSION 74.1)
FetchContent_Declare(
        inja
        GIT_REPOSITORY https://github.com/pantor/inja.git
        GIT_TAG v3.4.0
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS NAMES pantor # will invoke find_package first
)
FetchContent_MakeAvailable(base64 cpptrace hash-library rpp bshoshany-thread-pool nlohmann_json tsl-ordered-map fmtlog crossguid uriparser CURL ICU)


target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE
        bshoshany-thread-pool
        crossguid
        instinct::proto
        RPP::rpp
        fmt
        nlohmann_json
        icu::icu
        base64
        tsl::ordered_map
        fmtlog
        CURL::libcurl
        uriparser
        hash-library
        inja
        cpptrace
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


