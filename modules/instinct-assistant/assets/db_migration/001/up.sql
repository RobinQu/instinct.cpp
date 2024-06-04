
CREATE TABLE IF NOT EXISTS instinct_assistant(
    id VARCHAR PRIMARY KEY,
    object VARCHAR DEFAULT 'assistant' NOT NULL,
    model VARCHAR NOT NULL,
    name VARCHAR,
    description VARCHAR,
    instructions VARCHAR,
    metadata VARCHAR,
    temperature FLOAT DEFAULT 1 NOT NULL,
    top_p FLOAT DEFAULT 1 NOT NULL,
    response_format VARCHAR NOT NULL,
    tools VARCHAR,
    tool_resources VARCHAR,
    created_at TIMESTAMP DEFAULT now() NOT NULL,
    modified_at TIMESTAMP DEFAULT now() NOT NULL
);

CREATE TABLE IF NOT EXISTS instinct_thread(
    id VARCHAR PRIMARY KEY,
    object VARCHAR DEFAULT 'thread' NOT NULL,
    tool_resources VARCHAR,
    metadata VARCHAR,
    created_at TIMESTAMP DEFAULT now() NOT NULL,
    modified_at TIMESTAMP DEFAULT now() NOT NULL
);

CREATE TABLE IF NOT EXISTS instinct_thread_message(
    id VARCHAR PRIMARY KEY,
    object VARCHAR DEFAULT 'thread.message',
    created_at TIMESTAMP DEFAULT now() NOT NULL,
    modified_at TIMESTAMP DEFAULT now() NOT NULL,
    thread_id VARCHAR NOT NULL,
    status VARCHAR NOT NULL,
    incomplete_details VARCHAR,
    completed_at TIMESTAMP,
    incompleted_at TIMESTAMP,
    role VARCHAR NOT NULL,
    content VARCHAR NOT NULL,
    assistant_id VARCHAR,
    run_id VARCHAR,
    attachments VARCHAR,
    metadata VARCHAR
);

CREATE TABLE IF NOT EXISTS instinct_thread_run(
    id VARCHAR PRIMARY KEY,
    object VARCHAR DEFAULT 'thread.run' NOT NULL,
    created_at TIMESTAMP DEFAULT now() NOT NULL,
    modified_at TIMESTAMP DEFAULT now() NOT NULL,
    thread_id VARCHAR NOT NULL,
    assistant_id VARCHAR NOT NULL,
    status VARCHAR NOT NULL,
    required_action VARCHAR,
    last_error VARCHAR,
    expires_at TIMESTAMP,
    started_at TIMESTAMP,
    cancelled_at TIMESTAMP,
    failed_at TIMESTAMP,
    completed_at TIMESTAMP,
    incomplete_at TIMESTAMP,
    incomplete_details VARCHAR,
    model VARCHAR,
    instructions VARCHAR,
    usage VARCHAR,
    tools VARCHAR,
    temperature FLOAT,
    top_p FLOAT,
    max_prompt_tokens INTEGER,
    max_completion_tokens INTEGER,
    truncation_strategy VARCHAR NOT NULL,
    tool_choice VARCHAR,
    response_format VARCHAR NOT NULL,
    metadata VARCHAR
);

CREATE TABLE IF NOT EXISTS instinct_thread_run_step (
    id VARCHAR PRIMARY KEY,
    object VARCHAR DEFAULT 'thread.run.step' NOT NULL,
    created_at TIMESTAMP DEFAULT now() NOT NULL,
    modified_at TIMESTAMP DEFAULT now() NOT NULL,
    thread_id VARCHAR NOT NULL,
    run_id VARCHAR NOT NULL,
    type VARCHAR NOT NULL,
    status VARCHAR NOT NULL,
    step_details VARCHAR,
    last_error VARCHAR,
    expired_at TIMESTAMP,
    cancelled_at TIMESTAMP,
    failed_at TIMESTAMP,
    completed_at TIMESTAMP,
    metadata VARCHAR,
    usage VARCHAR
);

CREATE TABLE IF NOT EXISTS instinct_vector_store (
    id VARCHAR PRIMARY KEY,
    object VARCHAR DEFAULT 'vector_store' NOT NULL,
    created_at TIMESTAMP DEFAULT now() NOT NULL,
    modified_at TIMESTAMP DEFAULT now() NOT NULL,
    name VARCHAR NOT NULL,
    usage_bytes INTEGER DEFAULT 0 NOT NULL,
    status VARCHAR NOT NULL,
    expires_after VARCHAR,
    expires_at TIMESTAMP,
    last_active_at TIMESTAMP NOT NULL,
    metadata VARCHAR,
    summary VARCHAR
);

CREATE TABLE IF NOT EXISTS instinct_vector_store_file (
    file_id VARCHAR NOT NULL,
    object VARCHAR DEFAULT 'vector_store.file' NOT NULL,
    created_at TIMESTAMP DEFAULT now() NOT NULL,
    modified_at TIMESTAMP DEFAULT now() NOT NULL,
    vector_store_id VARCHAR NOT NULL,
    status VARCHAR NOT NULL,
    last_error VARCHAR,
    file_batch_id VARCHAR,
    summary VARCHAR,
    PRIMARY KEY (file_id, vector_store_id)
);


CREATE TABLE IF NOT EXISTS instinct_vector_store_file_batch (
    id VARCHAR PRIMARY KEY,
    object VARCHAR DEFAULT 'vector_store.file_batch' NOT NULL,
    created_at TIMESTAMP DEFAULT now() NOT NULL,
    modified_at TIMESTAMP DEFAULT now() NOT NULL,
    vector_store_id VARCHAR NOT NULL,
    status VARCHAR NOT NULL,
    last_error VARCHAR
);

CREATE TABLE IF NOT EXISTS instinct_file (
    id VARCHAR PRIMARY KEY,
    object VARCHAR DEFAULT 'file' NOT NULL,
    created_at TIMESTAMP DEFAULT now() NOT NULL,
    modified_at TIMESTAMP DEFAULT now() NOT NULL,
    bytes INT NOT NULL DEFAULT 0,
    filename VARCHAR NOT NULL,
    purpose VARCHAR NOT NULL
);


CREATE TABLE IF NOT EXISTS instinct_vector_store_metadata (
    instance_id VARCHAR PRIMARY KEY,
    metadata_schema VARCHAR NOT NULL,
    created_at TIMESTAMP DEFAULT now() NOT NULL,
    modified_at TIMESTAMP DEFAULT now() NOT NULL,
    embedding_table_name VARCHAR NOT NULL,
    custom VARCHAR
);