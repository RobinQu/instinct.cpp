from openai import OpenAI

client = OpenAI()
steps = client.beta.threads.runs.steps.list(
    thread_id="thread_hr57M2To5roEu61UyCXMW4n5",
    run_id="run_cGmFtvSDAVFFh8Kcfu5VMi8h"
)

print(steps.to_json())


for step in steps:
    if step.type == "message_creation":
        msg = client.beta.threads.messages.retrieve(
            message_id=step.step_details.message_creation.message_id,
            thread_id=step.thread_id
        )
        print(msg.to_json())
    if step.type == "tool_calls":
        print(step.step_details.to_json())

