import logging
logging.basicConfig(level=logging.DEBUG)


import pytest
import json
import time


def show_json(obj):
    print(json.loads(obj.model_dump_json()))


def wait_on_run(client, run, thread):
    while run.status == "queued" or run.status == "in_progress":
        run = client.beta.threads.runs.retrieve(
            thread_id=thread.id,
            run_id=run.id,
        )
        time.sleep(0.5)
    return run


def test_with_messages_only():
    """
    https://github.com/openai/openai-cookbook/blob/main/examples/Assistants_API_overview_python.ipynb
    :return:
    """
    from openai import OpenAI

    # default to read api key from env
    client = OpenAI(
        base_url="http://localhost:9091/v1"
    )

    assistant = client.beta.assistants.create(
        name="Math Tutor",
        instructions="You are a personal math tutor. Answer questions briefly, in a sentence or less.",
        model="gpt-3.5-turbo",
    )
    show_json(assistant)

    thread = client.beta.threads.create()
    show_json(thread)

    # Create a message to append to our thread
    message = client.beta.threads.messages.create(
        thread_id=thread.id, role="user", content="Could you explain this to me?"
    )
    show_json(message)

    # Execute our run
    run = client.beta.threads.runs.create(
        thread_id=thread.id,
        assistant_id=assistant.id,
    )
    # Wait for completion
    run = wait_on_run(client, run, thread)
    show_json(run)

    # Retrieve all the messages added after our last user message
    messages = client.beta.threads.messages.list(
        thread_id=thread.id, order="asc", after=message.id
    )
    show_json(messages)




