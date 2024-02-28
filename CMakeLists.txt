cmake_minimum_required(VERSION 3.27)
project(instinct)

set(CMAKE_CXX_STANDARD 20)


# specify default install location
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Specify install location." FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# defines CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_BINDIR and many other useful macros.
# see https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)

# control where libraries and executables are placed during the build.
# with the following settings executables are placed in <the top level of the
# build tree>/bin and libraries/archives in <top level of the build tree>/lib.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")

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

## fmt
add_subdirectory(thirdparty/fmt)
find_package(fmt)

## json
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(thirdparty/nlohmann_json)


## tensor
#add_subdirectory(thirdparty/xtl)
#add_subdirectory(thirdparty/xtensor)


## boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.84.0)

if (NOT Boost_FOUND)
    message(STATUS "add_subdirectory sub-directory of thirdparty/boost as Boost is not found")
#    set(Boost_VERBOSE ON)
#    set(BOOST_INCLUDE_LIBRARIES beast algorithm url)
#    set(ENV{BOOST_ROOT} "${PROJECT_SOURCE_DIR}/thirdparty/boost")
    add_subdirectory(thirdparty/boost EXCLUDE_FROM_ALL)
else ()
    message(STATUS "Boost version: ${Boost_VERSION}")
endif ()

## taskflow
set(TF_BUILD_TESTS OFF)
set(TF_BUILD_EXAMPLES OFF)
add_subdirectory(thirdparty/taskflow)

## rpp
add_subdirectory(thirdparty/ReactivePlusPlus)

## gtest
if(BUILD_TESTING)
#    find_package(GTest)
#    if(NOT GTEST_LIBRARIES)
        message(STATUS "add sub-directory of thirdpart/googtest as GTest is not found and BUILD_TESTING is enabled")
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        add_subdirectory(thirdparty/googletest)
#    else ()
#        message(STATUS "Found GTest, version=" ${GTest_VERSION} ", GTest libraries=" ${GTEST_LIBRARIES})
#    endif ()
endif ()


# project modules
add_subdirectory(modules/instinct-core)
add_subdirectory(modules/instinct-llm)
add_subdirectory(modules/instinct-retrieval)