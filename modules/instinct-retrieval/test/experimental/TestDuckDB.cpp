//
// Created by RobinQu on 2024/3/5.
//

#include <gtest/gtest.h>
#include <filesystem>
#include <duckdb.hpp>

#include "RetrievalGlobals.hpp"
#include "tools/TensorUtils.hpp"

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
        for(int i=0;i<n;i++) {
            vector<Value> vector_list_value;
            for(int j=0;j<d;j++) {
                vector_list_value.push_back(Value::FLOAT(xt[i*d+j]));
            }
            auto vector_value = Value::LIST(LogicalType::FLOAT, vector_list_value);
            auto result = prepared_statement->Execute(vector_value);
            if (auto error = result->GetErrorObject(); error.HasError()) {
                std::cout << error.Message() << std::endl;
            }
        }
    }

    void import_with_appender(Connection& con, const size_t nt, const size_t d, const float* xt) {
        // TODO Appender API is not suitable right now
        std::cout << "Appender started" << std::endl;
        Appender appender(con, "sift_1m");
        for(int i=0;i<nt;i++) {
            if (i==2000) break;
            appender.BeginRow();
            // appender.Append<std::nullptr_t>(nullptr);
            appender.Append<int>(i+1);
            vector<Value> vector_list_value;
            vector_list_value.reserve(d);
            for(int j=0;j<d;j++) {
                vector_list_value.push_back(Value::FLOAT(xt[i*d+j]));
            }
            auto vector_value = Value::LIST(LogicalType::FLOAT, vector_list_value);

            appender.Append(vector_value);
            // appender.AppendRow(i+1, vector_value);
            appender.EndRow();
        }
        appender.Close();
        std::cout << "Appender finished!" << std::endl;
    }



    TEST(DuckDB, TestBatchImport) {
        auto asset_dir_path = std::filesystem::current_path() /"modules/instinct-retrieval/test/_assets";

        // init db
            auto db_path =  asset_dir_path / "duckdb_test.db";
        DuckDB db(db_path.string());
        Connection con(db);
        std::vector<std::string> DSL = {
            "create or replace sequence sift_1m_id_seq start 1;",
            "create or replace table sift_1m (id BIGINT DEFAULT nextval('sift_1m_id_seq'), vector FLOAT[]);"
        };

        std::cout << "Prepare DB with SQLs:" << std::endl;
        for(const auto& sql: DSL) {
            std::cout << "-> " << sql << std::endl;
            con.Query(sql);
        }

        // read from sift1m

        auto sift_base_file = asset_dir_path/ "./sift1m/sift_base.fvecs";
        std::cout << "Reading SIFT-1M base from " << sift_base_file << std::endl;

        size_t d;
        size_t nt;
        float* xt = fvecs_read(sift_base_file.c_str(), &d, &nt);
        std::cout << "SIFT-1M base: n=" << nt << ",dim=" << d << std::endl;

        std::vector<std::string> DML = {
            "select * from sift_1m limit 20",
            "select count(*) from sift_1m"
        };

        size_t exepcted_number = 2000;
        auto count_result = con.Query(DML[1]);

        if(const auto count = count_result->GetValue(0, 0).GetValue<size_t>(); count != exepcted_number) {
            import_with_insert_statement(con, exepcted_number, d, xt);
        }

        std::cout << "Print random 20 records: " << std::endl;

        auto result = con.Query(DML[0]);
        // result->Print();
        for (int i = 0; i<result->RowCount(); i++) {
            auto id_value = result->GetValue(0, i);
            std::cout << "id=" << BigIntValue::Get(id_value) << std::endl;
            auto vector_value = result->GetValue(1, i);

            core::TensorUtils::PrintEmbedding("vector=", ListValue::GetChildren(vector_value));
        }




        // delete tensor data
        delete[] xt;
    }

}
