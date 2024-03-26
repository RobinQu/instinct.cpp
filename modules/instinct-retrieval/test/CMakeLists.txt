cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 20)

if(BUILD_TESTING)
    enable_testing()
endif()

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
#include_directories(${GTEST_INCLUDE_DIRS})
#if(NOT GTEST_LIBRARIES)
#    message(WARNING "gtest not found, download the library, build it and run cmake once again.")
#    return()
#endif()


file(GLOB_RECURSE TEST_SRC_FILES *.cpp)

include(FetchContent)
set(ASSETS_DIR "${CMAKE_CURRENT_BINARY_DIR}/_assets")
FetchContent_Declare(
        sift_base
        URL https://huggingface.co/datasets/qbo-odp/sift1m/resolve/main/sift_base.fvecs
        URL_HASH    SHA256=21f66e2975057b5728ba56de1c825bac4f4d89d596609ae985741c6242631816
        DOWNLOAD_DIR ${ASSETS_DIR}/sift1m/
        DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_Declare(
        sift_query
        URL https://huggingface.co/datasets/qbo-odp/sift1m/resolve/main/sift_query.fvecs
        URL_HASH    SHA256=f7fc9be140accdfd64116c2fa2365ecdb69b8f084970c6b0532db5ff79ac8fdc
        DOWNLOAD_DIR ${ASSETS_DIR}/sift1m/
        DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(sift_base sift_query)



# from list of files we'll create tests test_name.cpp -> test_name
foreach(_test_file ${TEST_SRC_FILES})
    get_filename_component(_test_name ${_test_file} NAME_WE)
    add_executable(${_test_name} ${_test_file})
    target_link_libraries(${_test_name} GTest::gtest GTest::gtest_main)
    target_link_libraries(${_test_name} ${LIBRARY_TARGET_NAME})
    add_test(${_test_name} ${_test_name})
    set_tests_properties(${_test_name} PROPERTIES TIMEOUT 120)

    # copy corpus test data to build tree
    add_custom_command(TARGET ${_test_name} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/_corpus/ $<TARGET_FILE_DIR:${_test_name}>/_corpus)

endforeach()
