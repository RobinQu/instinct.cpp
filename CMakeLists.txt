cmake_minimum_required(VERSION 3.26)
project(instinct VERSION 0.1.0)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# force cache value to update when building with submodules
# https://cmake.org/cmake/help/latest/policy/CMP0077.html
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# show progress
set(FETCHCONTENT_QUIET FALSE)

# specify default install location
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Specify install location." FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# defines CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_BINDIR and many other useful macros.
# see https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)

# add CTest
include(CTest)

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

# disable C and C++ compiler extensions.
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

# include additional cmake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# compilation options
option(BUILD_TESTING "Create tests using CMake" ON)
option(BUILD_SHARED_LIBS "Build libraries as shared as opposed to static" ON)
option(WITH_DUCKDB "Enable duckdb related classes" ON)
option(WITH_EXPRTK "Enable exprtk for LLM math" ON)
option(WITH_PDFIUM "Enable PDF parsing with PDFium" ON)
option(WITH_DUCKX "Enable DOCX parsing with duckx" ON)


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


# write config version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

# declare targets to be installed
install(TARGETS proto core llm transformer data retrieval
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)


# install header files
install(DIRECTORY ${PROJECT_BINARY_DIR}/modules/instinct-proto
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY
            ${PROJECT_SOURCE_DIR}/modules/instinct-core/include
            ${PROJECT_SOURCE_DIR}/modules/instinct-llm/include
            ${PROJECT_SOURCE_DIR}/modules/instinct-transformer/include
            ${PROJECT_SOURCE_DIR}/modules/instinct-data/include
            ${PROJECT_SOURCE_DIR}/modules/instinct-retrieval/include
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# write target file to lib/instinct/cmake folder
install(EXPORT ${PROJECT_NAME}_Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

# copy config files to lib/instinct/cmake folder
install(FILES
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
