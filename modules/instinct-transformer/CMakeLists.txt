cmake_minimum_required(VERSION 3.26)
project(instinct-transformer)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})

set(${LIBRARY_TARGET_NAME}_HDR
        include/config.hpp
        include/ops.hpp
        include/layers.hpp
        include/models.hpp
        include/tokenizer.hpp
        include/models/bge_ranker.hpp
        include/globals.hpp
        include/model_factory.hpp
        include/models/bge_embedding.hpp
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
        GIT_REPOSITORY https://github.com/RobinQu/ggml.git
        GIT_TAG 3d6595a67f4575665e16c29d4b7e610c25bf18ed
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