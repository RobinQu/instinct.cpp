# this file contains `find_package` for dependencies built by conan
find_package(Threads REQUIRED)

## gtest
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
endif ()

# deps for instinct-proto
find_package(protobuf REQUIRED)

# deps for instinct-core
find_package(inja REQUIRED)
find_package(bshoshany-thread-pool REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(RPP REQUIRED)
find_package(ICU REQUIRED)
find_package(tsl-ordered-map REQUIRED)
find_package(fmt REQUIRED)
find_package(fmtlog REQUIRED)
find_package(crossguid REQUIRED)
find_package(uriparser REQUIRED)
find_package(CURL REQUIRED)
find_package(hash-library REQUIRED)
find_package(base64 REQUIRED)
find_package(cpptrace REQUIRED)

# deps for instinct-llm
if (WITH_EXPRTK)
find_package(exprtk REQUIRED)
endif ()

# deps for instinct-transformer
find_package(llama-cpp REQUIRED)

# deps for instinct-data
if (WITH_DUCKDB)
find_package(duckdb REQUIRED)
endif ()

find_package(concurrentqueue REQUIRED)

# deps for instinct-retriever
if (WITH_DUCKX)
find_package(duckx REQUIRED)
endif ()

if (WITH_PDFIUM)
find_package(pdfium REQUIRED)
endif ()

# deps for instinct-server
find_package(httplib)

# deps for instinct-apps
find_package(CLI11)