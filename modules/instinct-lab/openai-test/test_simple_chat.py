import time
import json

from openai import OpenAI
from openai.types.beta import AssistantStreamEvent
from typing_extensions import override
from openai import AssistantEventHandler

client = OpenAI()

assistant = client.beta.assistants.create(
    name="assistant without tool use",
    instructions="You are a personal assistant to answer questions.",
    tools=[],
    model="gpt-3.5-turbo-0125"
)

thread = client.beta.threads.create()

message = client.beta.threads.messages.create(
    thread_id=thread.id,
    role="user",
    content="Why sky is blue"
)


class EventHandler(AssistantEventHandler):
    events: list[AssistantStreamEvent] = []

    def on_event(self, event: AssistantStreamEvent):
        self.events.append(event)


with open(f"output/test_simple_chat_{int(time.time())}.json", "w+") as f:
    # stream output
    event_handler = EventHandler()
    with client.beta.threads.runs.stream(
            thread_id=thread.id,
            assistant_id=assistant.id,
            event_handler=event_handler,
    ) as stream:
        stream.until_done()
    data = [e.to_dict(mode="json") for e in event_handler.events]
    json.dump(data, f)


with open(f"output/test_simple_chat_sync_{int(time.time())}.json", "w+") as f:
    # sync output
    run = client.beta.threads.runs.create_and_poll(
        thread_id=thread.id,
        assistant_id=assistant.id
    )
    data = {
        "run": run.to_dict(mode="json"),
        "messages": [m.to_dict(mode="json") for m in client.beta.threads.messages.list(
            thread_id=thread.id
        )],
        "run_steps": [step.to_dict(mode="json") for step in client.beta.threads.runs.steps.list(
            thread_id=thread.id,
            run_id=run.id
        )]
    }
    json.dump(data, f)
