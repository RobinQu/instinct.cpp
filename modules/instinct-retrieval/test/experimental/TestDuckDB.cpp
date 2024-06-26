//
// Created by RobinQu on 2024/3/5.
//

#include <gtest/gtest.h>
#include <filesystem>
#include <duckdb.hpp>

#include "RetrievalGlobals.hpp"
#include "tools/TensorUtils.hpp"
#include "../../../instinct-llm/include/instinct/LLMTestGlobals.hpp"

namespace INSTINCT_RETRIEVAL_NS::experimental {
    using namespace duckdb;
    /**
     * read sift-1m fvecs
     * copied from https://github.com/facebookresearch/faiss/blob/main/demos/demo_sift1M.cpp
     * @param fname absolute path to fvecs file
     * @param d_out dimension in fvecs
     * @param n_out number of vectors in fvecs
     * @return
     */
    float* fvecs_read(const char* fname, size_t* d_out, size_t* n_out) {
        FILE* f = fopen(fname, "r");
        if (!f) {
            fprintf(stderr, "could not open %s\n", fname);
            perror("");
            abort();
        }
        int d;
        fread(&d, 1, sizeof(int), f);
        assert((d > 0 && d < 1000000) || !"unreasonable dimension");
        fseek(f, 0, SEEK_SET);
        struct stat st;
        fstat(fileno(f), &st);
        size_t sz = st.st_size;
        assert(sz % ((d + 1) * 4) == 0 || !"weird file size");
        size_t n = sz / ((d + 1) * 4);

        *d_out = d;
        *n_out = n;
        float* x = new float[n * (d + 1)];
        size_t nr = fread(x, sizeof(float), n * (d + 1), f);
        assert(nr == n * (d + 1) || !"could not read whole file");

        // shift array to remove row headers
        for (size_t i = 0; i < n; i++)
            memmove(x + i * d, x + 1 + i * (d + 1), d * sizeof(*x));

        fclose(f);
        return x;
    }

    /**
     * read sift-1m ivecs
     * @param fname
     * @param d_out
     * @param n_out
     * @return
     */
    int* ivecs_read(const char* fname, size_t* d_out, size_t* n_out) {
        return (int*)fvecs_read(fname, d_out, n_out);
    }

    void import_with_insert_statement(Connection& con, const size_t n, const size_t d, const float* xt) {
        auto prepared_statement = con.Prepare("insert into sift_1m (vector) values ($1)");
        std::cout << "insert started, n=" << n << std::endl;
        long t1 = ChronoUtils::GetCurrentTimeMillis();
        for(int i=0;i<n;i++) {
            vector<duckdb::Value> vector_list_value;
            for(int j=0;j<d;j++) {
                vector_list_value.push_back(duckdb::Value::FLOAT(xt[i*d+j]));
            }
            auto array_value = duckdb::Value::ARRAY(LogicalType::FLOAT, vector_list_value);
            auto result = prepared_statement->Execute(array_value);
            if (auto error = result->GetErrorObject(); error.HasError()) {
                std::cout << error.Message() << std::endl;
            }
        }
        std::cout << "insert done, elapsed=" << (ChronoUtils::GetCurrentTimeMillis()-t1) << std::endl;
    }


    void import_range_with_appender(Connection& con, const size_t start, const size_t end, const size_t d, const float* xt) {
        std::cout << "--> Appender started, start=" << start << ", end=" << end << std::endl;

        long t1 = ChronoUtils::GetCurrentTimeMillis();
        Appender appender(con, "sift_1m");
        for(size_t i=start;i<end;i++) {
//            if (i==2000) break;
            appender.BeginRow();
            // appender.Append<std::nullptr_t>(nullptr);
//            appender.Append(++id_value);
            appender.Append<uint64_t>(i+1);
            vector<duckdb::Value> vector_list_value;
            vector_list_value.reserve(d);
            for(int j=0;j<d;j++) {
                vector_list_value.push_back(duckdb::Value::FLOAT(xt[i*d+j]));
            }
            auto array_value = duckdb::Value::ARRAY(LogicalType::FLOAT, vector_list_value);
            appender.Append(array_value);
            appender.EndRow();
        }
        appender.Close();
        std::cout << "--> Appender finished, elapsed=" << (ChronoUtils::GetCurrentTimeMillis() - t1) << std::endl;
    }

    void concurrent_import_with_appender(DuckDB& db, const unsigned int concurrency, const size_t batch_size, const size_t nt, const size_t d, const float* xt) {
        std::cout << "Import begins; n= " << nt << ",c=" << concurrency << ",batch_size=" << batch_size << std::endl;
        long t1 = ChronoUtils::GetCurrentTimeMillis();
        auto batch_num = (nt / batch_size) + 1;
        std::queue<std::thread> threads;
        for(int i=0;i<batch_num;i++) {
            threads.emplace([&,i]() {
                Connection con(db);
                import_range_with_appender(con, i*batch_size, std::min(nt, (i+1)*batch_size), d, xt);
            });
            if (threads.size() == concurrency) {
                threads.front().join();
                threads.pop();
            }
        }
        while(!threads.empty()) {
            threads.front().join();
            threads.pop();
        }
        std::cout << "Import finished, elapsed=" << (ChronoUtils::GetCurrentTimeMillis() - t1) << std::endl;
    }

    void import_with_appender(Connection& con, const size_t nt, const size_t d, const float* xt) {
//        static long id_value = 0;
        // TODO Appender API is not suitable right now
        std::cout << "Appender started, n=" << nt << std::endl;

        long t1 = ChronoUtils::GetCurrentTimeMillis();
        Appender appender(con, "sift_1m");
        for(int i=0;i<nt;i++) {
//            if (i==2000) break;
            appender.BeginRow();
            // appender.Append<std::nullptr_t>(nullptr);
//            appender.Append(++id_value);
            appender.Append<int>(i+1);
            vector<duckdb::Value> vector_list_value;
            vector_list_value.reserve(d);
            for(int j=0;j<d;j++) {
                vector_list_value.push_back(duckdb::Value::FLOAT(xt[i*d+j]));
            }
            auto array_value = duckdb::Value::ARRAY(LogicalType::FLOAT, vector_list_value);
            appender.Append(array_value);
            appender.EndRow();
        }
        appender.Close();
        std::cout << "Appender finished, elapsed=" << (ChronoUtils::GetCurrentTimeMillis() - t1) << std::endl;
    }

    TEST(DuckDB, TestBatchImport) {
        auto asset_dir_path = std::filesystem::current_path() / "_assets";

        // init db
        auto db_path =  INSTINCT_LLM_NS::ensure_random_temp_folder() / "duckdb_test.db";
        std::cout << "test db at " << db_path << std::endl;
        DuckDB db(db_path.string());
        Connection con(db);
        std::vector<std::string> DSL = {
            "create or replace sequence sift_1m_id_seq start 1;",
            "create or replace table sift_1m (id BIGINT DEFAULT nextval('sift_1m_id_seq'), vector FLOAT[128]);"
        };

        std::cout << "Prepare DB with SQLs:" << std::endl;
        for(const auto& sql: DSL) {
            std::cout << "-> " << sql << std::endl;
            con.Query(sql);
        }

        // read from sift1m

        auto sift_base_file = asset_dir_path/ "sift1m/sift_base.fvecs";
        std::cout << "Reading SIFT-1M base from " << sift_base_file << std::endl;

        size_t d;
        size_t nt;
        float* xt = fvecs_read(sift_base_file.c_str(), &d, &nt);
        std::cout << "SIFT-1M base: n=" << nt << ",dim=" << d << std::endl;

        std::vector<std::string> DML = {
            "select * from sift_1m limit 20",
            "select count(*) from sift_1m",
            "select id, list_cosine_similarity(vector, $1) as similarity from sift_1m order by similarity desc limit $2"
        };

//        size_t expected_number = 10000 * 10;
        size_t expected_number = 10000 * 5;
        auto count_result = con.Query(DML[1]);

        if(const auto count = count_result->GetValue(0, 0).GetValue<u_int32_t>(); count != expected_number) {
//            import_with_insert_statement(con, expected_number, d, xt);
//            import_with_appender(con, expected_number, d, xt);
            concurrent_import_with_appender(db, std::thread::hardware_concurrency(), 10000, expected_number, d, xt);
        }

        std::cout << "Print random 20 records: " << std::endl;

        auto result = con.Query(DML[0]);
        // result->Print();
        for (int i = 0; i<result->RowCount(); i++) {
            // do printing
            auto id_value = result->GetValue(0, i);
            std::cout << "id=" << BigIntValue::Get(id_value) << std::endl;
            auto vector_value = result->GetValue(1, i);

            core::TensorUtils::PrintEmbedding("vector=", ArrayValue::GetChildren(vector_value));

            // do topK recall
            auto search_statement = con.Prepare(DML[2]);
            long t1 = ChronoUtils::GetCurrentTimeMillis();
            auto search_result = search_statement->Execute(vector_value, 10);
            if (auto error=search_statement->GetErrorObject(); error.HasError()) {
                error.Throw("recall search failed");
            }
            std::cout << "recall done, elapsed=" << (ChronoUtils::GetCurrentTimeMillis()-t1) << std::endl;

            // print recall
            for(auto row: *search_result) {
                std::cout << "--> id=" << BigIntValue::Get(row.GetValue<int64_t>(0)) << std::endl;
                std::cout << "--> sim=" << FloatValue::Get(row.GetValue<float>(1)) << std::endl;
            }
        }

        // delete tensor data
        delete[] xt;
    }

    TEST(DuckDB, TestEscapeSQL) {
        DuckDB db(nullptr);
        Connection conn(db);

        const auto sqlline =  fmt::format(
            R"(select {} as json;)",
            "'" + StringUtils::EscapeSQLText(R"({"a":"{\"foo\": \"bar\"}"})") + "'"
            );
        const auto query_result = conn.Query(sqlline);
        query_result->Print();
        for (auto& r: *query_result) {
            const auto v = r.GetValue<std::string>(0);
            auto j1 = nlohmann::json::parse(v);
            ASSERT_TRUE(j1.contains("a"));
            auto j2 = nlohmann::json::parse(j1.at("a").get<std::string>());
            ASSERT_TRUE(j2.contains("foo"));
            ASSERT_EQ(j2.at("foo").get<std::string>(), "bar");
        }
    }

}
