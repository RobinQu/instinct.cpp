cmake_minimum_required(VERSION 3.27)
project(langchain_cpp)

set(CMAKE_CXX_STANDARD 17)


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
add_subdirectory(thirdparty/fmt)
find_package(fmt)

set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(thirdparty/nlohmann_json)

add_subdirectory(thirdparty/httplib)
add_subdirectory(thirdparty/xtl)
add_subdirectory(thirdparty/xtensor)


if(BUILD_TESTING)
    find_package(GTest)
    if(NOT GTEST_LIBRARIES)
        message(STATUS "add sub-directory of thirdpart/googtest as GTest is not found and BUILD_TESTING is enabled")
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        add_subdirectory(thirdparty/googletest)
    endif ()
endif ()

# project modules
add_subdirectory(modules/langchain-core)

#include(AddUninstallTarget)