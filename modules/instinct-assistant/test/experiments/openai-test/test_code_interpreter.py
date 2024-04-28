import time
import json

from openai import OpenAI
from openai.lib.streaming import AssistantEventHandler
from openai.types.beta import AssistantStreamEvent

client = OpenAI()


# Upload a file with an "assistants" purpose
file = client.files.create(
  file=open("groceries.xlsx", "rb"),
  purpose='assistants'
)

# Create an assistant using the file ID
assistant = client.beta.assistants.create(
  instructions="You are a personal math tutor. When asked a math question, write and run code to answer the question.",
  model="gpt-3.5-turbo-0125",
  tools=[{"type": "code_interpreter"}],
  tool_resources={
    "code_interpreter": {
      "file_ids": [file.id]
    }
  }
)

thread = client.beta.threads.create(
  messages=[
    {
      "role": "user",
      "content": "Please calculate total price of food items in the excel file?"
    }
  ]
)


class EventHandler(AssistantEventHandler):
    events: list[AssistantStreamEvent] = []

    def on_event(self, event: AssistantStreamEvent):
        self.events.append(event)


with open(f"output/test_code_interpreter_{int(time.time())}.json", "w+") as f:
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


with open(f"output/test_code_interpreter_sync_{int(time.time())}.json", "w+") as f:
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
