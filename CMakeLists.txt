cmake_minimum_required(VERSION 3.26)
project(instinct VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# force cache value to update when building with submodules
# https://cmake.org/cmake/help/latest/policy/CMP0077.html
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# show progress
Set(FETCHCONTENT_QUIET FALSE)

# specify default install location
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Specify install location." FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# defines CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_BINDIR and many other useful macros.
# see https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)

find_package(Threads REQUIRED)

# add CTest
include(CTest)

# control where libraries and executables are placed during the build.
# with the following settings executables are placed in <the top level of the
# build tree>/bin and libraries/archives in <top level of the build tree>/lib.
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")

# build position independent code.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# disable C and C++ compiler extensions.
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

# include additional cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


option(BUILD_TESTING "Create tests using CMake" ON)
option(BUILD_SHARED_LIBS "Build libraries as shared as opposed to static" ON)


# enable RPATH support for installed binaries and libraries
#include(AddInstallRPATHSupport)
#add_install_rpath_support(
#        BIN_DIRS "${CMAKE_INSTALL_FULL_BINDIR}"
#        LIB_DIRS "${CMAKE_INSTALL_FULL_LIBDIR}"
#        INSTALL_NAME_DIR "${CMAKE_INSTALL_FULL_LIBDIR}"
#        USE_LINK_PATH)

# encourage user to specify a build type (e.g. Release, Debug, etc.), otherwise set it to Release.
if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "Setting build type to 'Release' as none was specified.")
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
    endif()
endif()


# thirdparty dependencies

## json
#set(JSON_Install OFF CACHE INTERNAL "")
find_package(nlohmann_json)


## taskflow
#set(TF_BUILD_TESTS OFF)
#set(TF_BUILD_EXAMPLES OFF)
#add_subdirectory(thirdparty/taskflow)

## rpp
#add_subdirectory(thirdparty/ReactivePlusPlus EXCLUDE_FROM_ALL)
find_package(RPP REQUIRED)

## icu
find_package(ICU 74.1 REQUIRED)

## base64
find_package(base64)

## ordered-map
find_package(tsl-ordered-map REQUIRED)


## duckdb
find_package(duckdb REQUIRED)

## fmtlog and fmtlog
find_package(fmt)
find_package(fmtlog)

## crossguid
find_package(crossguid REQUIRED)

## uriparser
find_package(uriparser REQUIRED)

## libcurl
find_package(CURL)

## http server
find_package(httplib)

## pdfium for PDF parsing
find_package(pdfium REQUIRED)

## Tesseract for OCR
find_package(Tesseract REQUIRED)

## duckx for DOCX parsing
find_package(duckx REQUIRED)

find_package(exprtk REQUIRED)

## gtest
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
endif ()


# project modules
add_subdirectory(modules/instinct-proto)
add_subdirectory(modules/instinct-core)
add_subdirectory(modules/instinct-llm)
add_subdirectory(modules/instinct-retrieval)
add_subdirectory(modules/instinct-server)
add_subdirectory(modules/instinct-agent)
# examples
add_subdirectory(modules/instinct-examples/doc-agent)
add_subdirectory(modules/instinct-examples/quick-start)

#
#include(GNUInstallDirs)
#include(CMakePackageConfigHelpers)
#write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
#        VERSION ${PROJECT_VERSION}
#        COMPATIBILITY SameMajorVersion)
#
#configure_package_config_file(
#        "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
#        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
#        INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
#
#
#install(EXPORT InstinctTargets
#        FILE SITargets.cmake
#        NAMESPACE INSTINCT::
#        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
#
#install(FILES "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
#        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
#        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)