import os
import logging
import pathlib

import pytest
import json
import time
from openai import OpenAI
from openai.types.beta.threads import Text

logging.basicConfig(level=logging.DEBUG)

client = OpenAI(base_url="http://localhost:9091/v1")
MODEL_NAME = os.getenv("MODEL_NAME", "gpt-3.5-turbo")


def print_response_with_citation(message_content: Text):
    print(message_content)
    annotations = message_content.annotations
    citations = []
    for index, annotation in enumerate(annotations):
        message_content.value = message_content.value.replace(annotation.text, f"[{index}]")
        if file_citation := getattr(annotation, "file_citation", None):
            cited_file = client.files.retrieve(file_citation.file_id)
            citations.append(f"[{index}] {cited_file.filename} "
                             f"({annotation.start_index} to {annotation.end_index}): {file_citation.quote}")
    print("------------------------------")
    print(message_content.value)
    print("\n".join(citations))


def test_file_search_with_single_file():
    vector_store = client.beta.vector_stores.create(name="kb1")

    # Use the upload and poll SDK helper to upload the files, add them to the vector store,
    # and poll the status of the file batch for completion.
    dir_path = pathlib.Path(__file__).parent.resolve()
    vector_store_file = client.beta.vector_stores.files.upload_and_poll(
        vector_store_id=vector_store.id,
        file=open(dir_path / "munsey_magazine.txt", "rb")
    )

    # You can print the status and the file counts of the batch to see the result of this operation.
    print(vector_store_file.status)
    assert vector_store_file.status == "completed"

    # create assistant
    assistant = client.beta.assistants.create(
        name="Financial Analyst Assistant",
        instructions="You are assistant researcher. Use your knowledge to answer questions about the papers.",
        model="gpt-3.5-turbo",
        tools=[{"type": "file_search"}],
        tool_resources={"file_search": {"vector_store_ids": [vector_store.id]}}
    )

    # Create a thread and attach the file to the message
    thread = client.beta.threads.create(
        messages=[
            {
                "role": "user",
                "content": "What is Munsey's Magazine?",
            }
        ]
    )
    print(thread)

    # create run
    run = client.beta.threads.runs.create_and_poll(
        thread_id=thread.id, assistant_id=assistant.id
    )
    assert run.status == "completed"

    messages = list(client.beta.threads.messages.list(thread_id=thread.id, run_id=run.id))
    message_content = messages[0].content[0].text
    print_response_with_citation(message_content)


def test_file_search_with_multiple_files():
    vector_store = client.beta.vector_stores.create(name="kb2")

    file_paths = ["hygrophoraceae.txt", "marasmius_rotula.txt", "psathyrellaceae.txt"]
    dir_path = pathlib.Path(__file__).parent.resolve()
    file_streams = [open(dir_path / path, "rb") for path in file_paths]
    file_batch = client.beta.vector_stores.file_batches.upload_and_poll(
        vector_store_id=vector_store.id, files=file_streams
    )
    print(file_batch)
    assert file_batch.file_counts.in_progress == 0
    assert file_batch.file_counts.completed == file_batch.file_counts.total

    assistant = client.beta.assistants.create(
        name="Botanic Assistant",
        instructions="You are an expert biologist. "
                     "Use you knowledge base to answer questions.",
        model="gpt-4o",
        tools=[{"type": "file_search"}],
        tool_resources={"file_search": {"vector_store_ids": [vector_store.id]}}
    )
    print(assistant)

    thread = client.beta.threads.create(
        messages=[
            {
                "role": "user",
                "content": "Is Hygrophoraceae edible?"
            }
        ]
    )
    print(thread)

    run = client.beta.threads.runs.create_and_poll(
        thread_id=thread.id, assistant_id=assistant.id
    )
    assert run.status == "completed"

    messages = list(client.beta.threads.messages.list(thread_id=thread.id, run_id=run.id))

    message_content = messages[0].content[0].text
    print_response_with_citation(message_content)













