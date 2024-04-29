import json
import os.path
import time
from pathlib import Path

from openai import OpenAI

client = OpenAI()

# thread about travel expenses with column hints
# THREAD_ID = "thread_ZKcuR3Dj4rUnJKucq1jbRpAi"
# RUN_ID = "run_vbvEBpJu0GQfkELkYHYWJ30Z"


# thread about travel expenses without column hints, and it failed to answer
THREAD_ID = "thread_yI8ronxIqJRrcGGJvT3YSXVs"
RUN_ID = "run_nHjr9mg8UqB9tuz9VN84ezbo"


with open(f"output/test_get_run_steps_{int(time.time())}.json", "w+") as f:
    data = {
        "run": client.beta.threads.runs.retrieve(
            run_id=RUN_ID,
            thread_id=THREAD_ID
        ).to_dict(mode="json"),
        "steps": client.beta.threads.runs.steps.list(
            thread_id=THREAD_ID,
            run_id=RUN_ID
        ).to_dict(mode="json"),
        "messages": client.beta.threads.messages.list(
            thread_id=THREAD_ID
        ).to_dict(mode="json")
    }
    json.dump(data, f)

