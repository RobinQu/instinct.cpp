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

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

# disable C and C++ compiler extensions.
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

# include additional cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


option(BUILD_TESTING "Create tests using CMake" ON)
option(BUILD_SHARED_LIBS "Build libraries as shared as opposed to static" ON)

# encourage user to specify a build type (e.g. Release, Debug, etc.), otherwise set it to Release.
if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "Setting build type to 'Release' as none was specified.")
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
    endif()
endif()


# add dependencies
include(cmake/conan_dependencies.cmake)

# project modules
add_subdirectory(modules/instinct-proto)
add_subdirectory(modules/instinct-transformer)
add_subdirectory(modules/instinct-core)
add_subdirectory(modules/instinct-llm)
add_subdirectory(modules/instinct-retrieval)
add_subdirectory(modules/instinct-server)
add_subdirectory(modules/instinct-data)
add_subdirectory(modules/instinct-assistant)

# apps
add_subdirectory(modules/instinct-apps/doc-agent)
add_subdirectory(modules/instinct-apps/mini-assistant)
