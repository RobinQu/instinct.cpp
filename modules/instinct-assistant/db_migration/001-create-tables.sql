CREATE TABLE IF NOT EXISTS instinct_assistant(
       id VARCHAR PRIMARY KEY,
       model VARCHAR NOT NULL,
       name VARCHAR,
       description VARCHAR,
       instructions VARCHAR,
       metadata VARCHAR,
       temperature FLOAT,
       top_p FLOAT,
       response_format VARCHAR,
       tools VARCHAR,
       tool_resources VARCHAR,
       created_at INTEGER,
       modified_at INTEGER
);