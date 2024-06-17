cmake_minimum_required(VERSION 3.26)
project(instinct_retrieval)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})

set(${LIBRARY_TARGET_NAME}_HDR
        include/RetrievalGlobals.hpp
        include/ingestor/IIngestor.hpp
        include/retrieval/IRetriever.hpp
        include/retrieval/VectorStoreRetriever.hpp
        include/store/IVectorStore.hpp
        include/store/duckdb/DuckDBVectorStore.hpp
        include/chain/RAGChain.hpp
        include/store/IDocStore.hpp
        include/retrieval/MultiVectorRetriever.hpp
        include/retrieval/ChunkedMultiVectorRetriever.hpp
        include/retrieval/MultiQueryRetriever.hpp
        include/ingestor/BaseIngestor.hpp
        include/ingestor/SingleFileIngestor.hpp
        include/ingestor/DirectoryTreeIngestor.hpp
        include/store/duckdb/DuckDBDocStore.hpp
        include/RetrievalTestGlobals.hpp
        include/retrieval/BaseRetriever.hpp
        include/ingestor/PDFFileIngestor.hpp
        include/ingestor/DOCXFileIngestor.hpp
        include/store/duckdb/DuckDBDocWithEmbeddingStore.hpp
        include/ingestor/ParquetFileIngestor.hpp
        include/retrieval/MultiPathRetriever.hpp
        include/store/IVectorStoreOperator.hpp
        include/store/duckdb/DuckDBVectorStoreOperator.hpp
        include/store/VectorStoreMetadataDataMapper.hpp
        include/store/SQLBuilder.hpp
        include/chain/SummaryChain.hpp
        include/RetrieverObjectFactory.hpp
        include/chain/CitationAnnotatingChain.hpp
        include/retrieval/duckdb/DuckDBBM25Retriever.hpp
)

#set(${LIBRARY_TARGET_NAME}_SRC
#
#)

find_package(duckx REQUIRED)
find_package(pdfium REQUIRED)
find_package(Tesseract REQUIRED)



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
        icu::icu
        instinct::data
        Tesseract::libtesseract
        duckx::duckx
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