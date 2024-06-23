cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 20)

if(BUILD_TESTING)
    enable_testing()
endif()

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)


file(GLOB_RECURSE TEST_SRC_FILES *.cpp)

set(ASSETS_DIR "${CMAKE_CURRENT_BINARY_DIR}/_assets")
message(STATUS "ASSET_DIR: " ${ASSETS_DIR})

include(FetchContent)
FetchContent_Declare(
        bge-reranker-v2-m3.bin
        URL https://huggingface.co/robinqu/baai-bge-m3-guff/resolve/main/bge-reranker-m3-q4_1.bin?download=true
        URL_HASH    SHA256=2b0d1b8f41b8cfd6bc18aa90dd89671ba9aeb0d8121688f8b6a044634761de0c
        DOWNLOAD_NAME bge-reranker-v2-m3.bin
        DOWNLOAD_DIR ${ASSETS_DIR}/model_bins/
        DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_Declare(
        bge-m3e.bin
        URL https://huggingface.co/robinqu/baai-bge-m3-guff/resolve/main/bge-m3-q4_1.bin?download=true
        URL_HASH    SHA256=07312678bcb0a842c586f792a6d2a8f735fd3362de1d594969f687b17dc99c0d
        DOWNLOAD_NAME bge-m3e.bin
        DOWNLOAD_DIR ${ASSETS_DIR}/model_bins/
        DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(bge-reranker-v2-m3.bin bge-m3e.bin)


# from list of files we'll create tests test_name.cpp -> test_name
foreach(_test_file ${TEST_SRC_FILES})
    get_filename_component(_test_name ${_test_file} NAME_WE)
    add_executable(${_test_name} ${_test_file})
    target_link_libraries(${_test_name} GTest::gtest GTest::gtest_main)
    # link retrieval for testing purpose only
    target_link_libraries(${_test_name} ${LIBRARY_TARGET_NAME} instinct::transformer)
    add_test(${_test_name} ${_test_name})
    set_tests_properties(${_test_name} PROPERTIES TIMEOUT 30)
endforeach()
