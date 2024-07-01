cmake_minimum_required(VERSION 3.26)

# download CPM.cmake
file(
        DOWNLOAD
        https://raw.githubusercontent.com/vector-of-bool/cmrc/master/CMakeRC.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/CMakeRC.cmake
        EXPECTED_HASH SHA256=c1c80edbea8d7f4c4541d46d9ca53354f396a4b1e3ff412662613e78864d62b8
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CMakeRC.cmake)


file(GLOB_RECURSE ASSET_FILES *.sql)
cmrc_add_resource_library(instinct-assistants-resources ALIAS instinct::assistant::resources ${ASSET_FILES} NAMESPACE instinct::assistant)


