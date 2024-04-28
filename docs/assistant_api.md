# Assistant API

## Architecture overview

* `http-api` for restful endpoints
* `task-queue` for agent execution
* `tool-server`
  * multi-tenant vectordb for `file-search`
  * python sandbox for `code-interpreter` 


## http-api

OAS for assistant API can be found here: https://github.com/openai/openai-openapi

In first release of `mini-assistant`, following endpoints are supported:

* Assistant
  * POST `/assitants`
  * GET `/asstants/:assistant_id`
  * GET `/assistants`
  * POST `/asstants/:assistant_id`
  * DELETE `/assistants/:assitant_id`
* Thread
  * POST `/threads`
  * GET `/threads/:thread_id`
  * POST `/threads/:thread_id`
  * DELETE `/threads/:thread_id`
* Message
  * POST `/threads/:thread_id/messages`
  * GET `/threads/:thread_id/messages/:message_id`
  * GET `/threads/:thread_id/messages`
  * POST `/threads/:thread_id/messages/:message_id`
* Run
  * POST `/threads/:thread_id/runs`
  * POST `/threads/:thread_id/runs/:run_id`
  * GET `/threads/:thread_id/runs`
  * POST `/threads/:thread_id/runs/:rund_id/submit_tool_outputs`
  * POST `/threads/:thread_id/runs/:run_id/cancel`
* RunStep
  * GET `/threads/:thread_id/runs/:run_id/steps`
  * GET `/threads/:thread_id/runs/:run_id/steps/:step_id`
* File
  * GET `/files`
  * POST `/files`
  * DELETE `/files/:file_id`
  * GET `/files/:file_id`
  * GET `/files/:file_id/content`

## task-queue 

Task scheduling is needed in following sections:

* instinct-assistant
  * Periodic task to check status of run objects and run step objects in background.
  * FIFO queue for execution of run object, which handles agent execution. Only one running task is allowed for single run object.

### Worker queue for run objects

#### Preconditions

* run object is `queued` or `required_action`.
* all file resources needed are alive

#### Procedures

* Create `IAgentExecutor` instance with given tools setup
* Recover `AgentState` from database
* Run `IAgentExecutor::Stream` loop.
  * If run object is `cancelling` or `expired`, then stop. 
  * Update status of run object to `in_progress`.
  * if resolved step is agent thought, then
    * create a message with thought text.
    * create run step with status of `completed` and `step_details` with `message_creation` type.
    * create another run step object with status of `in_progress`, create a message with `step_details` with `tool_call` type.
      * if thought contains actions for function calls **stop**.
      * if thought contains actions for other tool uses, then continue. As `code_interpreter` and `file_search` is triggered automatically.
  * if resolved agent step is agent observation, then
    * update `step_details` of last run step object.
  * if resolved agent step is final message, then
    * create a message object containing the final message text.
    * create a run step with `step_details` of `message_creation` type and related `message_id`.
    * and **stop**
* if stopped and
  * status of run object is `cancelling`, then update status of run object to `canclled`.
  * final message is generated, update status of run object to `completed`. 
  * function tool calls are required, then update status of run object to `requires_action`.
  * error occurs during loop, update status of run object to `failed`.


#### Outcomes
* run object should be in intermediate status other than `queued`.
* run steps and generated messages are saved to database. 

### Background queue for run objects

#### Preconditions

* Only one thread is running this task across entire cluster.
* Variables: 
  * `RUN_TIMEOUT` defaults to 10min.


#### Procedures 

* Find all run objects that matches `modified_at < now() - RUN_TIMEOUT`.
* Loop run objects found
  * Find last run step that are `in_progress` and update them to status of `expired`.
  * Update run object to status of `expired`.

### Outcomes

* task and task steps that are timeout have been marked as `expired`.

## `tool-server`

### `file-search`

* duckdb implementation, mainly used for `mini-assistant`.
  * For Each file object we will generate one document table and embedding table.
  * Given only one `file_id` can be assigned to thread currently and only one process is accessing database files, we can manage all `VectorStorePtr` dynamically in memory.  e.g a map from `file_id` to `VectorStorePtr`.
* a more scalable solution involves standalone vector database.
  * A file object can be embedded and linked to a `collection`.
  * Mapping from `file_id` and `collection`'s id is required.

### `code-interpreter`

Prompting is straightforward. The challenge would the sandbox for Python scripts.

Basically two ways for sandboxes:

* Container with official Python binaries. 
  * [E2B sandbox](https://e2b.dev/docs/sandbox/overview).
  * https://github.com/significant-gravitas/autogpt/blob/master/autogpts/autogpt/autogpt/commands/execute_code.py
  * https://github.com/engineer-man/piston
  * https://github.com/StepicOrg/epicbox
* WebAssembly port of Python and run with a WebAssembly runtime. 
  * [Pyodide](https://pyodide.org/en/stable/index.html).
  * Sandbox engine using https://github.com/cohere-ai/cohere-terrarium


For `mini-assistant`, we will try script execution without sandbox first. And then try the WebAssembly way as it's possible to embed a working runtime without additional Docker or containerd setup.

For scalable solution, container-based solution is definitely the way to go as it can easily leverage cloud-native technologies like Kubernetes and KNative.



