cmake_minimum_required(VERSION 3.26)
project(instinct-transformer)

set(LIBRARY_TARGET_NAME transformer)

set(${LIBRARY_TARGET_NAME}_HDR
        include/instinct/transformer/config.hpp
        include/instinct/transformer/ops.hpp
        include/instinct/transformer/layers.hpp
        include/instinct/transformer/models.hpp
        include/instinct/transformer/tokenizer.hpp
        include/instinct/transformer/models/bge_ranker.hpp
        include/instinct/transformer_global.hpp
        include/instinct/transformer/model_factory.hpp
        include/instinct/transformer/models/bge_embedding.hpp
)

add_library(
        ${LIBRARY_TARGET_NAME} INTERFACE
        #        ${${LIBRARY_TARGET_NAME}_SRC}
        ${${LIBRARY_TARGET_NAME}_HDR}
        include/instinct/transformer_all.hpp
)

add_library(instinct::transformer
        ALIAS ${LIBRARY_TARGET_NAME})


target_include_directories(${LIBRARY_TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE
        llama-cpp::llama-cpp
        instinct::core
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")