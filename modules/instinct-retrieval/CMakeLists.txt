cmake_minimum_required(VERSION 3.27)
project(instinct_retrieval)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})

set(${LIBRARY_TARGET_NAME}_HDR
        src/RetrievalGlobals.hpp
        src/ingestor/IIngestor.hpp
        src/retrieval/IRetriever.hpp
        src/retrieval/VectorStoreRetriever.hpp
        src/store/IVectorStore.hpp
        src/store/duckdb/DuckDBVectorStore.hpp
        src/chain/RAGChain.hpp
        src/retrieval/DocumentUtils.hpp
        src/retrieval/IStatefulRetriever.hpp
        src/store/IDocStore.hpp
        src/retrieval/MultiVectorRetriever.hpp
        src/retrieval/ChunkedMultiVectorRetriever.hpp
        src/retrieval/AutoRetriever.hpp
        src/retrieval/MultiQueryRetriever.hpp

        src/output_parser/MultiLineTextOutputParser.hpp
        src/output_parser/MetadataQueryOutputParser.hpp
        src/ingestor/BaseIngestor.hpp
        src/ingestor/SingleFileIngestor.hpp
        src/ingestor/DirectoryTreeIngestor.hpp
        src/store/duckdb/DuckDBDocStore.hpp

        src/RetrievalTestGlobals.hpp
)

#set(${LIBRARY_TARGET_NAME}_SRC
#
#)

add_library(
        ${LIBRARY_TARGET_NAME} INTERFACE
#        ${${LIBRARY_TARGET_NAME}_SRC}
        ${${LIBRARY_TARGET_NAME}_HDR}
)


add_library(instinct::retrieval
        ALIAS ${LIBRARY_TARGET_NAME})


#set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES
#        #        VERSION ${${PROJECT_NAME}_VERSION}
#        PUBLIC_HEADER "${${LIBRARY_TARGET_NAME}_HDR}"
#)


target_include_directories(${LIBRARY_TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE
        instinct::proto
        instinct::core
        instinct::llm
        duckdb
)

install(TARGETS ${LIBRARY_TARGET_NAME}
        EXPORT  ${PROJECT_NAME}
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_TARGET_NAME}" COMPONENT dev)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")