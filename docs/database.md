# Database

Database related utilities are built into `instinct-data` module.

## Implementation Notes

### Date type manipulation

As is stated in [DuckDB official documentation](https://duckdb.org/docs/api/cpp.html), [TIMESTAMP](https://duckdb.org/docs/sql/data_types/timestamp)（with microsecond precision） in SQL is treated as `int64_t` in C++. Actually timestamp value is converted to epoch value in **nanosecond** precision.

> LogicalTypeId::DATE, LogicalTypeId::TIME, LogicalTypeId::INTEGER → int32_t
> LogicalTypeId::BIGINT, LogicalTypeId::TIMESTAMP → int64_t

So the best practice should be store datetime as `TIMESTAMP` in database and `int64_t` in C++ classes. 

However, some of restful APIs (esp. for OpenAI compatible APIs) contains timestamp in numeric form which has well known range limited for numbers in JSON format. 

Possible solutions are:

1. Store timestamps as `TIMESTAMP_S` with only second precision and map it to C++ data field of `int`, whose data range is safe in JSON.
2. Store timestamps as `int` in database and declare it `int` in C++ data field.
3. Store timestamps as `TIMESTAMP` with nano-second precision and do conversion during JSON serialization.


Discussions about these solutions:
* There is heavy use of `google::protobuf::utils::MessageToJsonString` for JSON serialization, but we've got no ways to modify how it works ([protobuf/issues/8952](https://github.com/protocolbuffers/protobuf/issues/8952)). A more sophisticated handling is required in `ProtobufUtils::Serialize` other than invoking `google::protobuf::utils::MessageToJsonString` directly.
* In terms of `TIMESTAMP_S`, it seems to be exclusive to DuckDB. It's not even supported in other popular database like MySQL, PostgresQL, which should be considered as data storage later by `instinct.cpp`.
* `int` storage in database has many [pitfalls](https://stackoverflow.com/questions/21499504/epoch-vs-date-and-time-in-mysql). e.g. Year 2038 problem, time zone, leap year, etc. 


Conclusion:
1. Store date as `TIMESTAMP` in database
2. Declare field as `int64` in protobuf message, so it's lossless for timestamps.
3. Custom serialization for presentation. (**WIP**)