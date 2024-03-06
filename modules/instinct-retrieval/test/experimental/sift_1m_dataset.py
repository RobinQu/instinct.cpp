# Read Sift dataset and conver to parquet file for DuckDB
# pip install pandas numpy pyarrow
import json

import numpy as np
import pandas as pd
import pyarrow as pa
import pyarrow.parquet as pq


def bvecs_read(fname):
    a = np.fromfile(fname, dtype=np.int32, count=1)
    b = np.fromfile(fname, dtype=np.uint8)
    d = a[0]
    return b.reshape(-1, d + 4)[:, 4:].copy()


def ivecs_read(fname):
    a = np.fromfile(fname, dtype='int32')
    d = a[0]
    return a.reshape(-1, d + 1)[:, 1:].copy()


def fvecs_read(fname):
    return ivecs_read(fname).view('float32')


def parquest_write_base(base_vectors: np.ndarray):
    # df = pd.DataFrame({
    #     "base": base_vectors
    # })
    # print(df.dtypes)
    # table = pa.Table.from_pandas(df=df, preserve_index=False)

    table = pa.table({"base": base_vectors})
    pq.write_table(table, "sift_1m_base.parquet")


def json_write_base(base_vectors: np.ndarray):
    json_data = {"vector": base_vectors.tolist()}
    with open("sift_1m_base.json", "w") as f:
        json.dump(json_data, f)


if __name__ == "__main__":
    data = fvecs_read("../_assets/sift1m/sift_base.fvecs")
    json_write_base(data)