include(FetchContent)

############################################################
#                                                          #
#                      instinct-proto                      #
#                                                          #
############################################################

# finding by config is required: https://github.com/protocolbuffers/protobuf/issues/12637
find_package(Protobuf 4.25 REQUIRED CONFIG)
if(Protobuf_FOUND)
    include_directories(${Protobuf_INCLUDE_DIRS})
    message(STATUS "Protobuf version : ${Protobuf_VERSION}")
    message(STATUS "Protobuf include path : ${Protobuf_INCLUDE_DIRS}")
    message(STATUS "Protobuf libraries : ${Protobuf_LIBRARIES}")
    message(STATUS "Protobuf compiler libraries : ${Protobuf_PROTOC_LIBRARIES}")
    message(STATUS "Protobuf lite libraries : ${Protobuf_LITE_LIBRARIES}")
    message(STATUS "Protobuf protoc : ${Protobuf_PROTOC_EXECUTABLE}")
endif()



############################################################
#                                                          #
#                      instinct-core                       #
#                                                          #
############################################################
FetchContent_Declare(
        RPP
        GIT_REPOSITORY https://github.com/victimsnino/ReactivePlusPlus.git
        GIT_TAG 973c278f6eb2302aa413e782822f7d7d1f4cb65e
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS NAMES RPP # will invoke find_package first
)
FetchContent_Declare(
        hash-library
        GIT_REPOSITORY https://github.com/stbrumme/hash-library.git
        GIT_TAG hash_library_v8
        GIT_SHALLOW 1
        PATCH_COMMAND git apply -q ${CMAKE_CURRENT_SOURCE_DIR}/patches/hash-library-fix-macos.patch || echo "Patch to hash_library failed"
        FIND_PACKAGE_ARGS # will invoke find_package first
)
FetchContent_Declare(
        base64
        GIT_REPOSITORY https://github.com/aklomp/base64.git
        GIT_TAG v0.5.2
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS # will invoke find_package first
)
FetchContent_Declare(
        BS_thread_pool
        GIT_REPOSITORY https://github.com/bshoshany/thread-pool.git
        GIT_TAG v4.1.0
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS # will invoke find_package first
)
FetchContent_Declare(
        cpptrace
        GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
        GIT_TAG v0.6.1
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS # will invoke find_package first
)
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS# will invoke find_package first
)
FetchContent_Declare(
        tsl-ordered-map
        GIT_REPOSITORY https://github.com/Tessil/ordered-map.git
        GIT_TAG v1.1.0
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS # will invoke find_package first
)
FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 10.2.1
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS # will invoke find_package first
)

# fix for fmtlog
find_package(fmtlog QUIET)
if (NOT fmtlog_FOUND)
    MESSAGE(STATUS "Post-process for fmtlog")
    FetchContent_Declare(
            fmtlog
            GIT_REPOSITORY https://github.com/MengRao/fmtlog.git
            GIT_TAG v2.2.1
            GIT_SHALLOW 1
            GIT_SUBMODULES "" # disable submodules
    )
    FetchContent_Populate(fmtlog)
    add_library(fmtlog ${fmtlog_SOURCE_DIR}/fmtlog.cc)
    target_link_libraries(fmtlog PRIVATE fmt)
    target_include_directories(fmtlog INTERFACE ${fmtlog_SOURCE_DIR})
    add_library(fmtlog::fmtlog ALIAS fmtlog)
endif ()

FetchContent_Declare(
        crossguid
        GIT_REPOSITORY https://github.com/graeme-hill/crossguid.git
        GIT_TAG v0.2.2
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS # will invoke find_package first
)
SET(URIPARSER_BUILD_TESTS OFF)
FetchContent_Declare(
        uriparser
        GIT_REPOSITORY https://github.com/uriparser/uriparser.git
        GIT_TAG uriparser-0.9.8
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS# will invoke find_package first
)


FetchContent_Declare(
        CURL
        GIT_REPOSITORY https://github.com/curl/curl.git
        GIT_TAG curl-8_8_0
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS # will invoke find_package first
)

# use pre-installed icu
find_package(ICU 73.2 REQUIRED
        COMPONENTS uc data i18n io
)
if (ICU_FOUND)
    message(STATUS "ICU_VERSION: " ${ICU_VERSION})
    message(STATUS "ICU_LIBRARIES: " ${ICU_LIBRARIES})
    message(STATUS "ICU_INCLUDE_DIRS: " ${ICU_INCLUDE_DIRS})
endif ()




FetchContent_Declare(
        inja
        GIT_REPOSITORY https://github.com/pantor/inja.git
        GIT_TAG v3.4.0
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS # will invoke find_package first
)

FetchContent_MakeAvailable(base64 cpptrace hash-library rpp nlohmann_json tsl-ordered-map crossguid uriparser BS_thread_pool fmt CURL)

if (base64_POPULATED)
    MESSAGE(STATUS "Post-process for base64")
    add_library(aklomp::base64 ALIAS base64)
endif ()

if (uriparser_POPULATED)
    MESSAGE(STATUS "Post-process for uriparser")
    add_library(uriparser::uriparser ALIAS uriparser)
endif ()

if (bs_thread_pool_POPULATED)
    MESSAGE(STATUS "Post-process for BS_thread_pool")
    # BS_thread_pool needs extra configurations after `FetchContent_MakeAvailable`
    add_library(BS_thread_pool INTERFACE)
    target_include_directories(BS_thread_pool INTERFACE ${bs_thread_pool_SOURCE_DIR}/include)
    message(STATUS "BS_thread_pool_SOURCE_DIR" ${bs_thread_pool_SOURCE_DIR})
    add_library(bshoshany-thread-pool::bshoshany-thread-pool ALIAS BS_thread_pool)
endif ()


# fix for hash-library
if (hash-library_POPULATED)
    MESSAGE(STATUS "Post-process for hash-library")
    file(GLOB_RECURSE HASHLIB_SRC_HEADERS ${hash-library_SOURCE_DIR}/*.h)
    file(GLOB_RECURSE HASHLIB_SRC_FILES ${hash-library_SOURCE_DIR}/*.cpp)
    add_library(hash-library ${HASHLIB_SRC_FILES} ${HASHLIB_SRC_HEADERS})
    target_include_directories(hash-library INTERFACE ${hash-library_SOURCE_DIR})
    add_library(hash-library::hash-library ALIAS hash-library)
endif ()

if (crossguid_POPULATED)
    MESSAGE(STATUS "Post-process for crossguid")
    # fix for crossguid as it does not export headers
    target_include_directories(xg INTERFACE ${crossguid_SOURCE_DIR})
    add_library(crossguid::crossguid ALIAS xg)
endif ()


# inja requires nlohmann_json to be available in advance
SET(INJA_USE_EMBEDDED_JSON OFF)
FetchContent_MakeAvailable(inja)


############################################################
#                                                          #
#                       instinct-llm                       #
#                                                          #
############################################################

FetchContent_Declare(
        exprtk
        GIT_REPOSITORY https://github.com/ArashPartow/exprtk.git
        GIT_TAG 0.0.2
        FIND_PACKAGE_ARGS # will invoke find_package first
)
FetchContent_MakeAvailable(exprtk)
if (exprtk_POPULATED)
    message(STATUS "exprtk_SOURCE_DIR: " ${exprtk_SOURCE_DIR})
    # fix for exprtk as it is not a CMake project
    add_library(exprtk INTERFACE ${exprtk_SOURCE_DIR}/exprtk.hpp)
    target_include_directories(exprtk INTERFACE $<BUILD_INTERFACE:${exprtk_SOURCE_DIR}>)
endif ()


############################################################
#                                                          #
#                    instinct-retriever                    #
#                                                          #
############################################################
FetchContent_Declare(
        duckx
        GIT_REPOSITORY https://github.com/amiremohamadi/DuckX.git
        GIT_TAG v1.2.2
        FIND_PACKAGE_ARGS # will invoke find_package first
)

FetchContent_Declare(
        pdfium
        URL https://github.com/bblanchon/pdfium-binaries/releases/latest/download/pdfium-mac-univ.tgz
        URL_HASH SHA256=e97039ff9eac8c215ed4bdf3d011192657f94aed4691083e3bf124eae70aefdb
)
FetchContent_MakeAvailable(duckx pdfium)

if (pdfium_POPULATED)
    # setup pdfium env according to their docs: https://github.com/bblanchon/pdfium-binaries
    message(STATUS "pdfium bianries" ${pdfium_SOURCE_DIR})
    SET(ENV{PDFium_DIR} ${pdfium_SOURCE_DIR})
    find_package(PDFium REQUIRED)
endif ()


############################################################
#                                                          #
#                      instinct-data                       #
#                                                          #
############################################################
FetchContent_Declare(
        concurrentqueue
        GIT_REPOSITORY https://github.com/cameron314/concurrentqueue.git
        GIT_TAG v1.0.4
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS # will invoke find_package first
)
FetchContent_Declare(
        duckdb
        GIT_REPOSITORY https://github.com/duckdb/duckdb.git
        GIT_TAG v1.0.0
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS # will invoke find_package first
)
FetchContent_MakeAvailable(concurrentqueue duckdb)

############################################################
#                                                          #
#                     instinct-server                      #
#                                                          #
############################################################
FetchContent_Declare(
        concurrentqueue
        GIT_REPOSITORY https://github.com/cameron314/concurrentqueue.git
        GIT_TAG v1.0.4
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS # will invoke find_package first
)
FetchContent_Declare(
        duckdb
        GIT_REPOSITORY https://github.com/duckdb/duckdb.git
        GIT_TAG v1.0.0
        GIT_SHALLOW 1
        FIND_PACKAGE_ARGS # will invoke find_package first
)
FetchContent_MakeAvailable(concurrentqueue duckdb)

############################################################
#                                                          #
#                     instinct-example                     #
#                                                          #
############################################################

FetchContent_Declare(
        CLI11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.4.2
        FIND_PACKAGE_ARGS # will invoke find_package first
)
FetchContent_MakeAvailable(CLI11)
