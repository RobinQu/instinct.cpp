cmake_minimum_required(VERSION 3.26)
project(instinct_core)

set(LIBRARY_TARGET_NAME core)
set(${LIBRARY_TARGET_NAME}_HDR
        include/instinct/tools/string_utils.hpp
        include/instinct/tools/http/http_utils.hpp
        include/instinct/tools/http_rest_client.hpp
        include/instinct/core_global.hpp
        include/instinct/tools/http/http_client_exception.hpp
        include/instinct/tools/tensor_utils.hpp
        include/instinct/tools/function_utils.hpp
        include/instinct/tools/assertions.hpp
        include/instinct/tools/chrono_utils.hpp
        include/instinct/functional/reactive_functions.hpp
        include/instinct/tools/codec_utils.hpp
        include/instinct/tools/http/http_client.hpp
        include/instinct/tools/http/curl_http_client.hpp
        include/instinct/functional/step_functions.hpp
        include/instinct/functional/context.hpp
        include/instinct/functional/json_context.hpp
        include/instinct/functional/runnable_chain.hpp
        include/instinct/functional/runnable.hpp
        include/instinct/tools/protobuf_utils.hpp
        include/instinct/functional/xn.hpp
        include/instinct/core_test_global.hpp
        include/instinct/tools/document_utils.hpp
        include/instinct/tools/metadata_schema_builder.hpp
        include/instinct/tools/system_utils.hpp
        include/instinct/tools/file_vault/file_vault.hpp
        include/instinct/tools/file_vault/file_system_file_vault.hpp
        include/instinct/tools/file_vault/http_url_resource_provider.hpp
        include/instinct/tools/file_vault/base_file_vault_resource_provider.hpp
        include/instinct/tools/hash_utils.hpp
        include/instinct/tools/io_utils.hpp
        include/instinct/tools/snowflake_id_generator.hpp
        include/instinct/tools/file_vault/temp_file.hpp
        include/instinct/tools/random_utils.hpp
        include/instinct/exception/instinct_exception.hpp
        include/instinct/exception/client_exception.hpp
        include/instinct/ioc/application_context.hpp
)

add_library(
        ${LIBRARY_TARGET_NAME} INTERFACE
        ${${LIBRARY_TARGET_NAME}_HDR}
        include/instinct/core_all.hpp
)
add_library(instinct::core
        ALIAS ${LIBRARY_TARGET_NAME})

message(STATUS "CURL_INCLUDE_DIR: " ${CURL_INCLUDE_DIR})
message(STATUS "ICU_INCLUDE_DIR: " ${ICU_INCLUDE_DIR})
message(STATUS "ICU_LIBRARIES: " ${ICU_LIBRARIES})

target_include_directories(${LIBRARY_TARGET_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${LIBRARY_TARGET_NAME} INTERFACE
        instinct::proto
        bshoshany-thread-pool::bshoshany-thread-pool
        crossguid::crossguid
        RPP::rpp
        fmt::fmt
        nlohmann_json::nlohmann_json
        icu::icu
        aklomp::base64
        tsl::ordered_map
        fmtlog::fmtlog
        CURL::libcurl
        uriparser::uriparser
        hash-library::hash-library
        pantor::inja
        cpptrace::cpptrace
)


if(BUILD_TESTING)
    add_subdirectory(test)
endif()


message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME}.")


