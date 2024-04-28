import json
import time
from typing_extensions import override

from openai import OpenAI
from openai.lib.streaming import AssistantEventHandler
from openai.types.beta import AssistantStreamEvent

client = OpenAI()

# Create an assistant using the file ID

assistant = client.beta.assistants.create(
  instructions="You are a weather bot. Use the provided functions to answer questions.",
  model="gpt-3.5-turbo-0125",
  tools=[
    {
      "type": "function",
      "function": {
        "name": "get_current_temperature",
        "description": "Get the current temperature for a specific location",
        "parameters": {
          "type": "object",
          "properties": {
            "location": {
              "type": "string",
              "description": "The city and state, e.g., San Francisco, CA"
            },
            "unit": {
              "type": "string",
              "enum": ["Celsius", "Fahrenheit"],
              "description": "The temperature unit to use. Infer this from the user's location."
            }
          },
          "required": ["location", "unit"]
        }
      }
    },
    {
      "type": "function",
      "function": {
        "name": "get_rain_probability",
        "description": "Get the probability of rain for a specific location",
        "parameters": {
          "type": "object",
          "properties": {
            "location": {
              "type": "string",
              "description": "The city and state, e.g., San Francisco, CA"
            }
          },
          "required": ["location"]
        }
      }
    }
  ]
)

thread = client.beta.threads.create()
message = client.beta.threads.messages.create(
  thread_id=thread.id,
  role="user",
  content="What's the weather in San Francisco today and the likelihood it'll rain?",
)

with open(f"output/test_function_call_{int(time.time())}.json", "w+") as f:
    class EventHandler(AssistantEventHandler):
        parent_handler = None
        events: list[AssistantStreamEvent] = []
        @override
        def on_event(self, event):
            self.events.append(event)
            if self.parent_handler:
                self.parent_handler.events.append(event)
            # Retrieve events that are denoted with 'requires_action'
            # since these will have our tool_calls
            if event.event == 'thread.run.requires_action':
                run_id = event.data.id  # Retrieve the run ID from the event data
                self.handle_requires_action(event.data, run_id)

        def handle_requires_action(self, data, run_id):
            tool_outputs = []

            for tool in data.required_action.submit_tool_outputs.tool_calls:
                if tool.function.name == "get_current_temperature":
                    tool_outputs.append({"tool_call_id": tool.id, "output": "57"})
                elif tool.function.name == "get_rain_probability":
                    tool_outputs.append({"tool_call_id": tool.id, "output": "0.06"})

            # Submit all tool_outputs at the same time
            self.submit_tool_outputs(tool_outputs, run_id)

        def submit_tool_outputs(self, tool_outputs, run_id):
            handler = EventHandler()
            handler.parent_handler = self
            # Use the submit_tool_outputs_stream helper
            with client.beta.threads.runs.submit_tool_outputs_stream(
                    thread_id=self.current_run.thread_id,
                    run_id=self.current_run.id,
                    tool_outputs=tool_outputs,
                    event_handler=handler
            ) as stream:
                for text in stream.text_deltas:
                    print(text, end="", flush=True)
                print()


    event_handler = EventHandler()
    with client.beta.threads.runs.stream(
            thread_id=thread.id,
            assistant_id=assistant.id,
            event_handler=event_handler
    ) as stream:
        stream.until_done()

    data = [e.to_dict(mode="json") for e in event_handler.events]
    json.dump(data, f)


with open(f"output/test_function_call_sync_{int(time.time())}.json", "w+") as f:
    run = client.beta.threads.runs.create_and_poll(
        thread_id=thread.id,
        assistant_id=assistant.id,
    )

    if run.status == 'completed':
        messages = client.beta.threads.messages.list(
            thread_id=thread.id
        )
        print(messages)
    else:
        print(run)

    # Define the list to store tool outputs
    tool_outputs = []

    # Loop through each tool in the required action section
    for tool in run.required_action.submit_tool_outputs.tool_calls:
        if tool.function.name == "get_current_temperature":
            tool_outputs.append({
                "tool_call_id": tool.id,
                "output": "57"
            })
        elif tool.function.name == "get_rain_probability":
            tool_outputs.append({
                "tool_call_id": tool.id,
                "output": "0.06"
            })

    # Submit all tool outputs at once after collecting them in a list
    if tool_outputs:
        try:
            run = client.beta.threads.runs.submit_tool_outputs_and_poll(
                thread_id=thread.id,
                run_id=run.id,
                tool_outputs=tool_outputs
            )
            print("Tool outputs submitted successfully.")
        except Exception as e:
            print("Failed to submit tool outputs:", e)
    else:
        print("No tool outputs to submit.")

    if run.status == 'completed':
        messages = client.beta.threads.messages.list(
            thread_id=thread.id
        )
        print(messages)
    else:
        print(run.status)

    # get all messages, run_steps
    data = {
        "run": run.to_dict(mode="json"),
        "run_steps": client.beta.threads.runs.steps.list(
            thread_id=thread.id,
            run_id=run.id
        ).to_dict(mode="json"),
        "messages": [msg.to_dict(mode="json") for msg in messages],
    }
    json.dump(data, f)


