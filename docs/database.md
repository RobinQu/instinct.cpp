# Database

Database related utilities are built into `instinct-data` module.

## Implementation Notes

### Date type manipulation

As is stated in [DuckDB official documentation](https://duckdb.org/docs/api/cpp.html), `TIMESTAMP` in SQL is treated as `int64_t` in C++. Actually timestamp value is converted to epoch value in **nanosecond** precision.

> LogicalTypeId::DATE, LogicalTypeId::TIME, LogicalTypeId::INTEGER → int32_t
> LogicalTypeId::BIGINT, LogicalTypeId::TIMESTAMP → int64_t

So the best practice should be store datetime as `TIMESTAMP` in database and `int64_t` in C++ classes.
