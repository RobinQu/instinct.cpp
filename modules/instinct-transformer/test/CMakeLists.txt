cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 20)

if(BUILD_TESTING)
    enable_testing()
endif()

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)


file(GLOB_RECURSE TEST_SRC_FILES *.cpp)

set(ASSETS_DIR "${CMAKE_CURRENT_BINARY_DIR}/_assets")

include(FetchContent)
FetchContent_Declare(
        bge-reranker-v2-m3.bin
        URL https://huggingface.co/robinqu/baai-bge-m3-guff/resolve/main/bge-reranker-v2-m3.bin?download=true
        URL_HASH    SHA256=b3e05dbe06c0aa52fd974d9c9dedbc51292b81f2f285d56113c060a0931a7f0f
        DOWNLOAD_NAME bge-reranker-v2-m3.bin
        DOWNLOAD_DIR ${ASSETS_DIR}/model_bins/
        DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(bge-reranker-v2-m3.bin)


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
