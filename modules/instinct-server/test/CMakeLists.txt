cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 20)

if(BUILD_TESTING)
    enable_testing()
endif()

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)


file(GLOB_RECURSE TEST_SRC_FILES *.cpp)


# from list of files we'll create tests test_name.cpp -> test_name
foreach(_test_file ${TEST_SRC_FILES})
    get_filename_component(_test_name ${_test_file} NAME_WE)
    add_executable(${_test_name} ${_test_file})
    target_link_libraries(${_test_name} GTest::gtest GTest::gtest_main)
    # link retrieval for testing purpose only
    target_link_libraries(${_test_name} ${LIBRARY_TARGET_NAME} instinct::retrieval)
    add_test(${_test_name} ${_test_name})
    set_tests_properties(${_test_name} PROPERTIES TIMEOUT 30)
endforeach()
