import logging
import pytest
import json
import time
from openai import OpenAI

logging.basicConfig(level=logging.DEBUG)


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


def test_with_multiple_thread_and_run():
    client = OpenAI(base_url="http://localhost:9091/v1")

    assistant = client.beta.assistants.create(
        name="Math Tutor",
        instructions="You are a personal math tutor. Answer questions briefly, in a sentence or less.",
        model="gpt-3.5-turbo",
    )

    MATH_ASSISTANT_ID = assistant.id  # or a hard-coded ID like "asst-..."

    def submit_message(assistant_id, thread, user_message):
        client.beta.threads.messages.create(
            thread_id=thread.id, role="user", content=user_message
        )
        return client.beta.threads.runs.create(
            thread_id=thread.id,
            assistant_id=assistant_id,
        )

    def get_response(thread):
        return client.beta.threads.messages.list(thread_id=thread.id, order="asc")

    def create_thread_and_run(user_input):
        thread = client.beta.threads.create()
        run = submit_message(MATH_ASSISTANT_ID, thread, user_input)
        return thread, run

    # Pretty printing helper
    def pretty_print(messages):
        print("# Messages")
        for m in messages:
            print(f"{m.role}: {m.content.text.value}")
        print()

    # Emulating concurrent user requests
    thread1, run1 = create_thread_and_run(
        "I need to solve the equation `3x + 11 = 14`. Can you help me?"
    )
    thread2, run2 = create_thread_and_run("Could you explain linear algebra to me?")
    thread3, run3 = create_thread_and_run("I don't like math. What can I do?")

    # Wait for Run 1
    run1 = wait_on_run(client, run1, thread1)
    pretty_print(get_response(thread1))

    # Wait for Run 2
    run2 = wait_on_run(client, run2, thread2)
    pretty_print(get_response(thread2))

    # Wait for Run 3
    run3 = wait_on_run(client, run3, thread3)
    pretty_print(get_response(thread3))

    # Thank our assistant on Thread 3 :)
    run4 = submit_message(MATH_ASSISTANT_ID, thread3, "Thank you!")
    run4 = wait_on_run(client, run4, thread3)
    pretty_print(get_response(thread3))








