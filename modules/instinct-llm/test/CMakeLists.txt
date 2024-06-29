cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 20)

if(BUILD_TESTING)
    enable_testing()
endif()

#include_directories(${GTEST_INCLUDE_DIRS})
#if(NOT GTEST_LIBRARIES)
#    message(WARNING "gtest not found, download the library, build it and run cmake once again.")
#    return()
#endif()


file(GLOB_RECURSE TEST_SRC_FILES *.cpp)

include(FetchContent)
set(ASSETS_DIR "${CMAKE_CURRENT_BINARY_DIR}/_assets")
FetchContent_Declare(
        cl100k_base
        URL  https://openaipublic.blob.core.windows.net/encodings/cl100k_base.tiktoken
        URL_HASH    SHA256=223921b76ee99bde995b7ff738513eef100fb51d18c93597a113bcffe865b2a7
        DOWNLOAD_DIR ${ASSETS_DIR}/bpe_ranks/
        DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_Declare(
        gpt2_bpe
        URL  https://openaipublic.blob.core.windows.net/gpt-2/encodings/main/vocab.bpe
        URL_HASH    SHA256=1ce1664773c50f3e0cc8842619a93edc4624525b728b188a9e0be33b7726adc5
        DOWNLOAD_DIR ${ASSETS_DIR}/bpe_ranks/
        DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_Declare(
        gpt2_encoder_json
        URL  https://openaipublic.blob.core.windows.net/gpt-2/encodings/main/encoder.json
        URL_HASH    SHA256=196139668be63f3b5d6574427317ae82f612a97c5d1cdaf36ed2256dbf636783
        DOWNLOAD_DIR ${ASSETS_DIR}/bpe_ranks/
        DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_MakeAvailable(cl100k_base gpt2_bpe gpt2_encoder_json)


# from list of files we'll create tests test_name.cpp -> test_name
foreach(_test_file ${TEST_SRC_FILES})
    get_filename_component(_test_name ${_test_file} NAME_WE)
    add_executable(${_test_name} ${_test_file})
    target_link_libraries(${_test_name} GTest::gtest GTest::gtest_main)
    target_link_libraries(${_test_name} instinct::llm)
    add_test(${_test_name} ${_test_name})
    set_tests_properties(${_test_name} PROPERTIES LABELS "unit_test;llm")
    set_tests_properties(${_test_name} PROPERTIES TIMEOUT 120)
endforeach()
