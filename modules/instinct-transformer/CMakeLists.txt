cmake_minimum_required(VERSION 3.26)
project(instinct-transformer)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})

set(${LIBRARY_TARGET_NAME}_HDR
        include/instinct/config.hpp
        include/instinct/ops.hpp
        include/instinct/layers.hpp
        include/instinct/models.hpp
        include/instinct/tokenizer.hpp
        include/instinct/models/bge_ranker.hpp
        include/instinct/globals.hpp
        include/instinct/model_factory.hpp
        include/instinct/models/bge_embedding.hpp
)

add_library(
        ${LIBRARY_TARGET_NAME} INTERFACE
        #        ${${LIBRARY_TARGET_NAME}_SRC}
        ${${LIBRARY_TARGET_NAME}_HDR}
)

add_library(instinct::transformer
        ALIAS ${LIBRARY_TARGET_NAME})


include(FetchContent)
FetchContent_Declare(
        ggml
        GIT_REPOSITORY https://github.com/ggerganov/ggml.git
        GIT_TAG f5c9599
        FIND_PACKAGE_ARGS # will invoke find_package first
)
#set(GGML_PERF ON)
FetchContent_MakeAvailable(ggml)


target_include_directories(${LIBRARY_TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE
        ggml
        instinct::core
)

install(TARGETS ${LIBRARY_TARGET_NAME}
        EXPORT  ${PROJECT_NAME}
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/instinct" COMPONENT dev)


install(DIRECTORY include/
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/instinct"
        COMPONENT instinct_transformer
)


if(BUILD_TESTING)
    add_subdirectory(test)
endif()

message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")