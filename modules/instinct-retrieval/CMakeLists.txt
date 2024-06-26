cmake_minimum_required(VERSION 3.26)
project(instinct_retrieval)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})

set(${LIBRARY_TARGET_NAME}_HDR
        include/instinct/RetrievalGlobals.hpp
        include/instinct/ingestor/IIngestor.hpp
        include/instinct/retrieval/IRetriever.hpp
        include/instinct/retrieval/VectorStoreRetriever.hpp
        include/instinct/store/IVectorStore.hpp
        include/instinct/store/duckdb/DuckDBVectorStore.hpp
        include/instinct/chain/RAGChain.hpp
        include/instinct/store/IDocStore.hpp
        include/instinct/retrieval/MultiVectorRetriever.hpp
        include/instinct/retrieval/ChunkedMultiVectorRetriever.hpp
        include/instinct/retrieval/MultiQueryRetriever.hpp
        include/instinct/ingestor/BaseIngestor.hpp
        include/instinct/ingestor/SingleFileIngestor.hpp
        include/instinct/ingestor/DirectoryTreeIngestor.hpp
        include/instinct/store/duckdb/DuckDBDocStore.hpp
        include/instinct/RetrievalTestGlobals.hpp
        include/instinct/retrieval/BaseRetriever.hpp
        include/instinct/ingestor/PDFFileIngestor.hpp
        include/instinct/ingestor/DOCXFileIngestor.hpp
        include/instinct/store/duckdb/DuckDBDocWithEmbeddingStore.hpp
        include/instinct/ingestor/ParquetFileIngestor.hpp
        include/instinct/retrieval/MultiPathRetriever.hpp
        include/instinct/store/IVectorStoreOperator.hpp
        include/instinct/store/duckdb/DuckDBVectorStoreOperator.hpp
        include/instinct/store/VectorStoreMetadataDataMapper.hpp
        include/instinct/store/SQLBuilder.hpp
        include/instinct/chain/SummaryChain.hpp
        include/instinct/RetrieverObjectFactory.hpp
        include/instinct/chain/CitationAnnotatingChain.hpp
        include/instinct/retrieval/duckdb/DuckDBBM25Retriever.hpp
        include/instinct/retrieval/ParentChildRetriever.hpp
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
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE
        instinct::proto
        instinct::core
        instinct::llm
        pdfium::pdfium
        duckx::duckx
        instinct::data
)

install(TARGETS ${LIBRARY_TARGET_NAME}
        EXPORT  ${PROJECT_NAME}
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/instinct" COMPONENT dev)

install(DIRECTORY include/
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/instinct"
        COMPONENT instinct_retrieval
)


if(BUILD_TESTING)
    add_subdirectory(test)
endif()

message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")