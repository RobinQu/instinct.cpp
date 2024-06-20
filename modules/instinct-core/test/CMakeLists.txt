cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 20)

#FetchContent_Declare(
#        GTest
#        GIT_REPOSITORY https://github.com/google/googletest.git
#        GIT_TAG Version v1.14.0
#        FIND_PACKAGE_ARGS NAMES GTest # will invoke find_package first
#)
#FetchContent_MakeAvailable(GTest)

if(BUILD_TESTING)
    enable_testing()
endif()

#find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
#include_directories(${GTEST_INCLUDE_DIRS})
#if(NOT GTEST_LIBRARIES)
#    message(WARNING "gtest not found, download the library, build it and run cmake once again.")
#    return()
#endif()


file(GLOB_RECURSE TEST_SRC_FILES *.cpp)


# from list of files we'll create tests test_name.cpp -> test_name
foreach(_test_file ${TEST_SRC_FILES})
    get_filename_component(_test_name ${_test_file} NAME_WE)
    add_executable(${_test_name} ${_test_file})
    target_link_libraries(${_test_name} GTest::gtest GTest::gtest_main)
    target_link_libraries(${_test_name} ${LIBRARY_TARGET_NAME})
    add_test(${_test_name} ${_test_name})
    set_tests_properties(${_test_name} PROPERTIES TIMEOUT 30)
endforeach()
