cmake_minimum_required(VERSION 3.26)
project(instinct_retrieval)

set(LIBRARY_TARGET_NAME lib${PROJECT_NAME})

set(${LIBRARY_TARGET_NAME}_HDR
        include/instinct/retrieval_global.hpp
        include/instinct/retrieval/retriever.hpp
        include/instinct/retrieval/vector_store_retriever.hpp
        include/instinct/store/vector_store.hpp
        include/instinct/store/duckdb/duckdb_vector_store.hpp
        include/instinct/chain/rag_chain.hpp
        include/instinct/store/doc_store.hpp
        include/instinct/retrieval/multi_vector_retriever.hpp
        include/instinct/retrieval/chunked_multi_vector_retriever.hpp
        include/instinct/retrieval/multi_query_retriever.hpp
        include/instinct/ingestor/ingestor.hpp
        include/instinct/ingestor/single_file_ingestor.hpp
        include/instinct/ingestor/directory_tree_ingestor.hpp
        include/instinct/store/duckdb/duckdb_doc_store.hpp
        include/instinct/retrieval_test_global.hpp
        include/instinct/retrieval/base_retriever.hpp
        include/instinct/ingestor/pdf_file_ingestor.hpp
        include/instinct/ingestor/docx_file_ingestor.hpp
        include/instinct/store/duckdb/duckdb_doc_with_embedding_store.hpp
        include/instinct/ingestor/parquet_file_ingestor.hpp
        include/instinct/retrieval/multi_path_retriever.hpp
        include/instinct/store/vector_store_operator.hpp
        include/instinct/store/duckdb/duckdb_vector_store_operator.hpp
        include/instinct/store/vector_store_metadata_data_mapper.hpp
        include/instinct/store/sql_builder.hpp
        include/instinct/chain/summary_chain.hpp
        include/instinct/retriever_object_factory.hpp
        include/instinct/chain/citation_annotating_chain.hpp
        include/instinct/retrieval/duckdb/duckdb_bm25_retriever.hpp
        include/instinct/retrieval/parent_child_retriever.hpp
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