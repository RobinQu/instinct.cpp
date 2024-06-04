//
// Created by RobinQu on 2024/4/29.
//

#ifndef RUNOBJECTHANDLER_HPP
#define RUNOBJECTHANDLER_HPP

#include <utility>

#include "AssistantGlobals.hpp"
#include "LLMObjectFactory.hpp"
#include "agent/executor/BaseAgentExecutor.hpp"
#include "task_scheduler/ThreadPoolTaskScheduler.hpp"
#include "agent/patterns/openai_tool/OpenAIToolAgentExecutor.hpp"
#include "assistant/v2/service/IVectorStoreService.hpp"
#include "assistant/v2/toolkit/SummaryGuidedFileSearch.hpp"
#include "toolkit/LocalToolkit.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;

    /**
     * Used for runtime option overrides
     */
    struct AgentOverrides {
        std::optional<std::string> model_name;
        std::optional<std::string> instructions;
        std::optional<float> top_p;
        std::optional<float> temperature;
    };

    using AgentExecutorProvider = std::function<AgentExecutorPtr(const LLMProviderOptions& llm_options, const AgentExecutorOptions& options)>;

    /**
     * Task handler for run objects using `OpenAIToolAgentExecutor`.
     */
    class RunObjectTaskHandler final: public CommonTaskScheduler::ITaskHandler {
        RunServicePtr run_service_;
        MessageServicePtr message_service_;
        AssistantServicePtr assistant_service_;
        LLMProviderOptions llm_provider_options_;
        AgentExecutorOptions agent_executor_options_;
        RetrieverOperatorPtr retriever_operator_;
        VectorStoreServicePtr vector_store_service_;
        ThreadServicePtr thread_service_;

    public:
        static inline std::string CATEGORY = "run_object";

        RunObjectTaskHandler(RunServicePtr run_service, MessageServicePtr message_service,
            AssistantServicePtr assistant_service, RetrieverOperatorPtr retriever_operator,
            VectorStoreServicePtr vector_store_service, ThreadServicePtr thread_service, LLMProviderOptions llm_provider_options,
            AgentExecutorOptions agent_executor_options)
            : run_service_(std::move(run_service)),
              message_service_(std::move(message_service)),
              assistant_service_(std::move(assistant_service)),
              llm_provider_options_(std::move(llm_provider_options)),
              agent_executor_options_(std::move(agent_executor_options)),
              retriever_operator_(std::move(retriever_operator)),
              vector_store_service_(std::move(vector_store_service)),
              thread_service_(std::move(thread_service)) {
        }

        bool Accept(const ITaskScheduler<std::string>::Task &task) override {
            return task.category == CATEGORY;
        }

        void Handle(const ITaskScheduler<std::string>::Task &task) override {
            trace_span span {"RunObjectTaskHandler::Handle"};
            RunObject run_object;
            ProtobufUtils::Deserialize(task.payload, run_object);

            if (!CheckPreconditions_(run_object)) {
                LOG_WARN("Precondition failure for run object: {}", run_object.ShortDebugString());
                return;
            }

            // update run object with `status` and `started_at`
            using namespace std::chrono_literals;
            ModifyRunRequest modify_run_request;
            modify_run_request.set_run_id(run_object.id());
            modify_run_request.set_thread_id(run_object.thread_id());
            modify_run_request.set_status(RunObject_RunObjectStatus_in_progress);
            modify_run_request.set_started_at(ChronoUtils::GetCurrentEpochMicroSeconds());
            // expect to expire at 10 miniutes later
            modify_run_request.set_expires_at(ChronoUtils::GetLaterEpoch<std::chrono::microseconds>(10min));
            if(!run_service_->ModifyRun(modify_run_request)) {
                LOG_ERROR("Illegal response for updating run object: {}", run_object.ShortDebugString());
                return;
            }

            const auto state_opt = RecoverAgentState(run_object);
            if (!state_opt) {
                LOG_ERROR("Failed to recover state with run object: {}", run_object.ShortDebugString());
                return;
            }

            const auto executor = BuildAgentExecutor_(run_object);
            if(!executor) {
                LOG_ERROR("Failed to create agent executor with run object: {}", run_object.ShortDebugString());
                return;
            }

            // execute possible steps
            executor->Stream(state_opt.value())
                | rpp::operators::as_blocking()
                | rpp::operators::subscribe([&](const AgentState& current_state) {
                    // respond to state changes
                    const auto last_step = current_state.previous_steps().rbegin();
                    if (last_step->has_thought()) {
                        if (last_step->thought().has_continuation()
                            && last_step->thought().continuation().has_tool_call_message()) { // should contain thought of calling code interpreter and file search, which are invoked automatically
                            OnAgentContinuation(last_step->thought().continuation(), run_object);
                            return;
                        }

                        if (last_step->thought().has_pause()
                            && last_step->thought().pause().has_tool_call_message()) { // should contain thought of calling function tools
                            OnAgentPause_(last_step->thought().pause(), run_object);
                            return;
                        }

                        if(last_step->thought().has_finish()) { // finish message
                            OnAgentFinish_(last_step->thought().finish(), run_object);
                            return;
                        }
                    }

                    if (last_step->has_observation() && last_step->observation().tool_messages_size() > 0) {
                        // 1. function tool call results are submitted
                        // 2. or only contain tool calls for code interpreter and file search
                        OnAgentObservation_(last_step->observation(), run_object);
                        return;
                    }

                    LOG_WARN("Illegal message from agent: {}", last_step->ShortDebugString());
                }, [&](const std::exception_ptr& e) {
                    // translate exception to AgentFinish
                    AgentFinish agent_finish;
                    agent_finish.set_is_failed(true);
                    RunEarlyStopDetails run_early_stop_details;
                    try {
                        std::rethrow_exception(e);
                    } catch (const ClientException& client_exception) {
                        run_early_stop_details.mutable_error()->set_type(invalid_request_error);
                        run_early_stop_details.mutable_error()->set_message(client_exception.what());
                    } catch (const InstinctException& instinct_exception) {
                        run_early_stop_details.mutable_error()->set_type(server_error);
                        run_early_stop_details.mutable_error()->set_message(instinct_exception.what());
                    } catch (const std::runtime_error& runtime_error) {
                        run_early_stop_details.mutable_error()->set_type(server_error);
                        run_early_stop_details.mutable_error()->set_message(runtime_error.what());
                    } catch (...) {
                        run_early_stop_details.mutable_error()->set_type(server_error);
                        run_early_stop_details.mutable_error()->set_message("Uncaught exception");
                    }
                    agent_finish.mutable_details()->PackFrom(run_early_stop_details);
                    OnAgentFinish_(agent_finish, run_object);
                });
        }


        [[nodiscard]] std::optional<AgentState> RecoverAgentState(const RunObject& run_object) const {
            AgentState state;
            if (!LoadAgentStateFromRun_(run_object, state)) {
                LOG_ERROR("Cannot load agent state from run object: {}", run_object.ShortDebugString());
                return std::nullopt;
            }

            // load function tools
            if (!LoadFunctionTools_(run_object, state)) {
                LOG_ERROR("Cannot load function tool schamas for agent state from run object: {}", run_object.ShortDebugString());
                return std::nullopt;
            }

            return state;
        }


    private:

        /**
         * Check following conditions:
         * 1. run object is in status of `queued`
         * 2. file resources referenced are valid
         * @param run_object
         * @return
         */
        // ReSharper disable once CppMemberFunctionMayBeStatic
        [[nodiscard]] bool CheckPreconditions_(const RunObject& run_object) const { // NOLINT(*-convert-member-functions-to-static)
            return run_object.status() == RunObject_RunObjectStatus_queued;
        }

        /**
         * Just return `OpenAIToolAgentExecutor`
         * @param run_object
         * @return
         */
        [[nodiscard]] AgentExecutorPtr BuildAgentExecutor_(const RunObject& run_object) const {
            GetAssistantRequest get_assistant_request;
            get_assistant_request.set_assistant_id(run_object.assistant_id());
            const auto assistant_obj = assistant_service_->RetrieveAssistant(get_assistant_request);
            if (!assistant_obj) {
                LOG_ERROR("Failed to get assistant object with id: {}", run_object.assistant_id());
                return nullptr;
            }

            // load model options from user objects
            const auto chat_model = LLMObjectFactory::CreateChatModel(llm_provider_options_);
            ModelOverrides model_overrides;
            if (StringUtils::IsNotBlankString(run_object.model())) {
                model_overrides.model_name = run_object.model();
            } else if(StringUtils::IsNotBlankString(assistant_obj->model())) {
                model_overrides.model_name = assistant_obj->model();
            }
            if (run_object.has_temperature()) {
                model_overrides.temperature = run_object.temperature();
            } else if (assistant_obj->has_temperature()) {
                model_overrides.temperature = assistant_obj->temperature();
            }
            if (run_object.has_top_p()) {
                model_overrides.top_p = run_object.top_p();
            } else if(assistant_obj->has_top_p()) {
                model_overrides.top_p = assistant_obj->top_p();
            }
            model_overrides.stop_words =  {"<END_OF_PLAN>", "<END_OF_RESPONSE>"};
            chat_model->Configure(model_overrides);

            // load instructions from user objects
            auto agent_options = agent_executor_options_;
            std::optional<std::string> instructions;
            if (StringUtils::IsNotBlankString(run_object.instructions())) {
                instructions = run_object.instructions();
            } else if(StringUtils::IsNotBlankString(assistant_obj->instructions())) {
                instructions = assistant_obj->instructions();
            }
            // TODO configure method for agent executor
            if(instructions) {
                agent_options.llm_compiler.joiner_input_parser.instructions = instructions.value();
            }
            StopPredicate stop_predicate = [&](const AgentState& state, AgentStep& step) {
                return CheckRunObjectForExecution_(run_object.thread_id(), run_object.id(), state, step);
            };

            // build file search tools
            GetThreadRequest get_thread_request;
            get_thread_request.set_thread_id(run_object.thread_id());
            const auto thread_obj = thread_service_->RetrieveThread(get_thread_request);
            assert_true(thread_obj, fmt::format("Thread is missing. id={}", run_object.thread_id()));
            std::vector<FunctionToolPtr> tools;
            AddFileSearchTools_(assistant_obj.value(), run_object, thread_obj.value(), tools);

            // create agent executor
            return  LLMObjectFactory::CreateAgentExecutor(
                agent_options,
                chat_model,
                stop_predicate,
                tools.empty() ? std::vector<FunctionToolkitPtr> {} : std::vector {CreateLocalToolkit(tools)}
                );
        }

        void AddFileSearchTools_(
            const AssistantObject& assistant_object,
            const RunObject& run_object,
            const ThreadObject& thread_object,
            std::vector<FunctionToolPtr>& tools
            ) const {
            bool has_file_search = false;
            for(const auto& tool: assistant_object.tools()) {
                if (tool.type() == file_search) {
                    has_file_search = true;
                }
            }
            // if file_search is not enabled by assistant, let's exit
            if(!has_file_search) return;
            assert_true(assistant_object.tool_resources().file_search().vector_store_ids_size()>0, "should have at least one VectorStore");

            std::vector<std::string> vs_id_list;
            // add VS on assistant
            vs_id_list.push_back(assistant_object.tool_resources().file_search().vector_store_ids(0));
            // add VS on thread if any
            if (thread_object.tool_resources().has_file_search() && thread_object.tool_resources().file_search().vector_store_ids_size()>0) {
                vs_id_list.push_back(thread_object.tool_resources().file_search().vector_store_ids(0));
            }
            // find files
            const auto related_files = vector_store_service_->ListAllVectorStoreObjectFiles(vs_id_list);
            const auto retriever = retriever_operator_->GetStatelessRetriever(vs_id_list);
            const auto file_search_tool = CreateSummaryGuidedFileSearch(
                CreateLocalRankingModel(ModelType::BGE_M3_RERANKER),
                retriever,
                related_files
                );
            tools.push_back(file_search_tool);
        }

        /**
         * Predicate if early stop is required for run object
         * @param thread_id
         * @param run_id
         * @param state
         * @param step
         * @return
         */
        bool CheckRunObjectForExecution_(const std::string& thread_id, const std::string& run_id, const AgentState& state, AgentStep& step) const {
            GetRunRequest get_run_request;
            get_run_request.set_run_id(run_id);
            get_run_request.set_thread_id(thread_id);
            const auto get_run_resp = run_service_->RetrieveRun(get_run_request);
            RunEarlyStopDetails run_early_stop_details;
            auto *finish = step.mutable_thought()->mutable_finish();
            if (get_run_resp.has_value()) {
                if (get_run_resp->status() == RunObject_RunObjectStatus_cancelling
                    || get_run_resp->status() == RunObject_RunObjectStatus_cancelled
                    // it's unlikely for a `canceled` run object to be passed in, but just in case of that, we choose to set it to `cancelled` again.
                    ) {
                    // save to any field
                    finish->mutable_details()->PackFrom(run_early_stop_details);
                    finish->set_is_cancelled(true);
                    return true;
                }
                if (get_run_resp->status() == RunObject_RunObjectStatus_expired) {
                    finish->set_is_expired(true);
                    // save to any field
                    step.mutable_thought()->mutable_finish()->mutable_details()->PackFrom(run_early_stop_details);
                    return true;
                }
            } else {
                run_early_stop_details.mutable_error()->set_type(invalid_request_error);
                finish->set_is_failed(true);
                step.mutable_thought()->mutable_finish()->mutable_details()->PackFrom(run_early_stop_details);
                return false;
            }
            return false;
        }


        /**
         * After continuation message is generated,
         * 1. create run step of `message_creation` if `OpenAIToolAgentContinuation.tool_call_message` contains content string
         * 2. create run step object with type of `tool_calls` and status of `in_progress` if `OpenAIToolAgentContinuation.tool_call_message` contains non-empty `tool_calls`.
         * 3. update run object to status of `in_progress`.
         * @param agent_continuation
         * @param run_object
         */
        void OnAgentContinuation(const AgentContinuation& agent_continuation, const RunObject& run_object) const {
            LOG_INFO("OnAgentContinuation Start, agent_continuation={}", agent_continuation.ShortDebugString());

            RunStepObject run_step_object;
            run_step_object.set_thread_id(run_object.thread_id());
            run_step_object.set_run_id(run_object.id());
            run_step_object.set_type(RunStepObject_RunStepType_tool_calls);
            run_step_object.set_assistant_id(run_object.assistant_id());
            run_step_object.set_status(RunStepObject_RunStepStatus_in_progress);
            auto* step_details = run_step_object.mutable_step_details();
            // save custom data
            step_details->mutable_custom()->CopyFrom(agent_continuation.custom());

            // create step with message if tool message has content string
            if (StringUtils::IsNotBlankString(agent_continuation.tool_call_message().content())) {
                if (!CreateMessageStep_(agent_continuation.tool_call_message().content(), run_object)) {
                    LOG_ERROR("Illegal reponse for creating step object with message. tool_call_message={}, run_object={}", agent_continuation.tool_call_message().DebugString(), run_object.ShortDebugString());
                    return;
                }
            }

            // create step with tool step if tool message contains tool call requests
            if (agent_continuation.tool_call_message().tool_calls_size() > 0) {
                // TODO support code interpreter and file serach
                for(const auto& tool_request: agent_continuation.tool_call_message().tool_calls()) {
                    auto* tool_call_detail = step_details->mutable_tool_calls()->Add();
                    tool_call_detail->set_id(tool_request.id());
                    tool_call_detail->set_type(function);
                    auto* function_call = tool_call_detail->mutable_function();
                    function_call->set_name(tool_request.function().name());
                    function_call->set_arguments(tool_request.function().arguments());
                }
                if (!run_service_->CreateRunStep(run_step_object)) {
                    LOG_ERROR("Illegal response for creating run step object: {}", run_step_object.ShortDebugString());
                    return;
                }
            }

            if(!UpdateRunObjectStatus(run_step_object.thread_id(), run_step_object.run_id(), RunObject_RunObjectStatus_in_progress)) {
                LOG_ERROR("Illegal response for updating run object: {}", run_object.ShortDebugString());
                return;
            }

            LOG_INFO("OnAgentContinuation Done, agent_continuation={}", agent_continuation.ShortDebugString());
        }

        [[nodiscard]] std::optional<RunObject> UpdateRunObjectStatus(const std::string& thread_id, const std::string& run_id, const RunObject_RunObjectStatus status) const {
            ModifyRunRequest modify_run_request;
            modify_run_request.set_run_id(run_id);
            modify_run_request.set_thread_id(thread_id);
            modify_run_request.set_status(status);
            return run_service_->ModifyRun(modify_run_request);
        }

        [[nodiscard]] std::optional<std::pair<RunStepObject, MessageObject>> CreateMessageStep_(const std::string& content, const RunObject& run_object) const {
            // TODO need transaction
            CreateMessageRequest create_message_request;
            create_message_request.set_thread_id(run_object.thread_id());
            create_message_request.set_role(assistant);
            create_message_request.set_content(content);
            create_message_request.set_assistant_id(run_object.assistant_id());
            create_message_request.set_run_id(run_object.id());

            const auto message_object = message_service_->CreateMessage(create_message_request);
            if (!message_object.has_value()) {
                LOG_ERROR("Cannot create message for this step. run_object={}, create_message_request={}", run_object.ShortDebugString(), create_message_request.ShortDebugString());
                return std::nullopt;
            }

            RunStepObject run_step_object;
            run_step_object.set_run_id(run_object.id());
            run_step_object.set_thread_id(run_object.thread_id());
            run_step_object.set_assistant_id(run_object.assistant_id());
            run_step_object.set_type(RunStepObject_RunStepType_message_creation);
            run_step_object.mutable_step_details()->set_type(RunStepObject_RunStepType_message_creation);
            run_step_object.set_status(RunStepObject_RunStepStatus_completed);
            run_step_object.mutable_step_details()->mutable_message_creation()->set_message_id(message_object->id());
            const auto create_run_step_resp = run_service_->CreateRunStep(run_step_object);
            if(!create_run_step_resp) {
                LOG_ERROR("Cannot create run step, run_step_object={}", run_step_object.ShortDebugString());
                return std::nullopt;
            }

            return std::pair {create_run_step_resp.value(), message_object.value()};
        }


        /**
         * After pause message is generated,
         * 1. update `step_details` of run step object with completed tool call results.
         * 2. update `run_object` with status of `required_action` and correct content of `required_actions`
         * @param agent_pause
         * @param run_object
         */
        void OnAgentPause_(const AgentPause& agent_pause, const RunObject& run_object) {
            LOG_INFO("OnAgentPause Start, agent_pause={}", agent_pause.ShortDebugString());
            const auto last_run_step_opt = RetrieveLastRunStep_(run_object);
            if (!last_run_step_opt) {
                LOG_ERROR("Cannot find last run step for run object: {}", run_object.ShortDebugString());
                return;
            }

            // update run step
            ModifyRunStepRequest modify_run_step_request;
            modify_run_step_request.set_run_id(run_object.id());
            modify_run_step_request.set_step_id(last_run_step_opt->id());
            modify_run_step_request.set_thread_id(run_object.thread_id());
            auto* step_details = modify_run_step_request.mutable_step_details();
            step_details->CopyFrom(last_run_step_opt->step_details());
            // update custom data
            step_details->mutable_custom()->CopyFrom(agent_pause.custom());

            ModifyRunRequest modify_run_request;
            modify_run_request.set_run_id(run_object.id());
            modify_run_request.set_thread_id(run_object.thread_id());
            modify_run_request.set_status(RunObject_RunObjectStatus_requires_action);
            modify_run_request.mutable_required_action()->set_type(RunObject_RequiredActionType_submit_tool_outputs);

            // find unfinished tool calls
            for(const auto& tool_call: agent_pause.tool_call_message().tool_calls()) {
                bool done = false;
                for(const auto& tool_message: agent_pause.completed()) {
                    done = tool_message.tool_call_id() == tool_call.id();
                }
                if (!done) {
                    auto* tool_request = modify_run_request.mutable_required_action()->mutable_submit_tool_outputs()->mutable_tool_calls()->Add();
                    tool_request->set_type(ToolCallObjectType::function);
                    tool_request->mutable_function()->set_name(tool_call.function().name());
                    tool_request->mutable_function()->set_arguments(tool_call.function().arguments());
                    tool_request->set_id(tool_call.id());
                }
            }

            // update finished tool calls
            for(const auto& tool_message: agent_pause.completed()) {
                for(auto& tool_call_in_step_detail: *step_details->mutable_tool_calls()) {
                    if (tool_call_in_step_detail.id() == tool_message.tool_call_id()) {
                        if (tool_call_in_step_detail.type() == AssistantToolType::function) {
                            tool_call_in_step_detail.mutable_function()->set_output(tool_message.content());
                        }
                    }
                    // TODO support code interpreter and file serach
                }
            }

            if (!run_service_->ModifyRunStep(modify_run_step_request)) {
                LOG_ERROR("Illegal response for updating run step object: {}", modify_run_step_request.ShortDebugString());
                return;
            }

            // update run object
            if(!run_service_->ModifyRun(modify_run_request)) {
                LOG_ERROR("Illegal response for update run object: {}", run_object.ShortDebugString());
                return;
            }
            LOG_INFO("OnAgentPause Done, agent_pause={}", agent_pause.ShortDebugString());
        }

        /**
         * 1. update run object to status of `completed`
         * 2. update of run step object with `step_details` of completed tool call results and status of `completed`
         * @param observation
         * @param run_object
         */
        void OnAgentObservation_(const AgentObservation& observation, const RunObject& run_object) {
            LOG_INFO("OnAgentObservation Start, observation={}", observation.ShortDebugString());

            const auto last_run_step = RetrieveLastRunStep_(run_object);
            if (!last_run_step) {
                LOG_ERROR("Cannot find last run step for run object: {}", run_object.ShortDebugString());
                return;
            }

            // update status of run object to `in_progress`
            if (!UpdateRunObjectStatus(run_object.thread_id(), run_object.id(), RunObject_RunObjectStatus_in_progress)) {
                LOG_ERROR("Cannot update run object. run_object={}", run_object.ShortDebugString());
                return;
            }

            // update run step with tool call outputs
            ModifyRunStepRequest modify_run_step_request;
            modify_run_step_request.set_run_id(run_object.id());
            modify_run_step_request.set_step_id(last_run_step->id());
            modify_run_step_request.set_thread_id(run_object.thread_id());
            modify_run_step_request.set_status(RunStepObject_RunStepStatus_completed);
            auto* step_details = modify_run_step_request.mutable_step_details();
            step_details->CopyFrom(last_run_step->step_details());
            // update custom data
            step_details->mutable_custom()->CopyFrom(observation.custom());

            // TODO support code interpreter and file serach
            for(const auto& tool_message: observation.tool_messages()) {
                for(int i=0;i<step_details->tool_calls_size();++i) {
                    if (auto* tool_call = step_details->mutable_tool_calls(i);
                        tool_call->id() == tool_message.tool_call_id()) {
                        tool_call->mutable_function()->set_output(tool_message.content());
                        }
                }
            }
            if (!run_service_->ModifyRunStep(modify_run_step_request)) {
                LOG_ERROR("Illegal response for updating run step object: {}", modify_run_step_request.ShortDebugString());
                return;
            }

            LOG_INFO("OnAgentObservation Done, observation={}", observation.ShortDebugString());
        }

        /**
         * if it's successfully finished:
         * 1. update current run step object with status of `completed`
         * 2. create a new run step object with type of `message_creation`.
         * 3. create a message containing result content in the current thread.
         * 4. update run object with status of `completed`.
         *
         * if it's finished with exception
         * 1. update run step object with status of `failed` and correct content of `last_error`.
         * 2. update run object with status of `failed`.
         *
         * @param finish_message
         * @param run_object
         */
        void OnAgentFinish_(const AgentFinish& finish_message, const RunObject& run_object) {
            LOG_INFO("OnAgentFinish Start, finish_message={}", finish_message.ShortDebugString());
            // TODO needs transaction

            ModifyRunRequest modify_run_request;
            modify_run_request.set_run_id(run_object.id());
            modify_run_request.set_thread_id(run_object.thread_id());

            // finish with no previous step
            if (const auto last_run_step = RetrieveLastRunStep_(run_object); !last_run_step) {
                if (finish_message.is_failed()) {
                    modify_run_request.set_status(RunObject_RunObjectStatus_failed);
                    modify_run_request.set_failed_at(ChronoUtils::GetCurrentEpochMicroSeconds());
                } else if (finish_message.is_cancelled()) {
                    modify_run_request.set_status(RunObject_RunObjectStatus_cancelled);
                    modify_run_request.set_cancelled_at(ChronoUtils::GetCurrentEpochMicroSeconds());
                } else if (finish_message.is_expired()) {
                    modify_run_request.set_status(RunObject_RunObjectStatus_expired);
                } else {
                    modify_run_request.set_status(RunObject_RunObjectStatus_completed);
                    modify_run_request.set_completed_at(ChronoUtils::GetCurrentEpochMicroSeconds());
                    if(!CreateMessageStep_(finish_message.response(), run_object)) {
                        LOG_ERROR("Cannot create message for final answer. run_object={}", run_object.ShortDebugString());
                        return;
                    }
                }
            } else { // another branch which has previous steps
                ModifyRunStepRequest modify_run_step_request;
                modify_run_step_request.set_run_id(run_object.id());
                modify_run_step_request.set_step_id(last_run_step->id());
                modify_run_step_request.set_thread_id(run_object.thread_id());
                modify_run_step_request.mutable_step_details()->CopyFrom(last_run_step->step_details());
                // update custom data
                modify_run_step_request.mutable_step_details()->mutable_custom()->CopyFrom(finish_message.custom());

                if (finish_message.is_failed()) {
                    // update last run step object with status of `failed`
                    modify_run_step_request.set_failed_at(ChronoUtils::GetCurrentEpochMicroSeconds());
                    modify_run_step_request.set_status(RunStepObject_RunStepStatus_failed);

                    if (finish_message.has_details() && finish_message.details().Is<RunEarlyStopDetails>()) { // find error data from details
                        RunEarlyStopDetails run_early_stop_details;
                        finish_message.details().UnpackTo(&run_early_stop_details);
                        if (run_early_stop_details.has_error()) {
                            modify_run_step_request.mutable_last_error()->CopyFrom(run_early_stop_details.error());
                        }
                    }
                    if (!modify_run_step_request.has_last_error()) {
                        // fallback to invalid_request_error
                        LOG_WARN("last_error is not set correctly. run_object={}", run_object.ShortDebugString());
                        modify_run_step_request.mutable_last_error()->set_type(invalid_request_error);
                    }
                    // update run object with status of `failed`
                    modify_run_request.set_status(RunObject_RunObjectStatus_failed);
                    modify_run_request.set_failed_at(ChronoUtils::GetCurrentEpochMicroSeconds());
                } else if (finish_message.is_cancelled()) {
                    modify_run_step_request.set_status(RunStepObject_RunStepStatus_cancelled);
                    modify_run_step_request.set_cancelled_at(ChronoUtils::GetCurrentEpochMicroSeconds());
                    modify_run_request.set_status(RunObject_RunObjectStatus_cancelled);
                    modify_run_request.set_cancelled_at(ChronoUtils::GetCurrentEpochMicroSeconds());
                } else if (finish_message.is_expired()) {
                    modify_run_step_request.set_expired_at(ChronoUtils::GetCurrentEpochMicroSeconds());
                    modify_run_step_request.set_status(RunStepObject_RunStepStatus_expired);
                    modify_run_request.set_status(RunObject_RunObjectStatus_expired);
                } else {
                    // update last run step object with status of `completed`
                    modify_run_step_request.set_completed_at(ChronoUtils::GetCurrentEpochMicroSeconds());
                    modify_run_step_request.set_status(RunStepObject_RunStepStatus_completed);
                    modify_run_request.set_status(RunObject_RunObjectStatus_completed);
                    modify_run_request.set_completed_at(ChronoUtils::GetCurrentEpochMicroSeconds());

                    // create message step
                    LOG_DEBUG("Final answer resolved: response={}, run_id={}", finish_message.response(), run_object.id());
                    if (!CreateMessageStep_(finish_message.response(), run_object)) {
                        LOG_ERROR("Failed to create message and run step. modify_run_request={}", modify_run_request.ShortDebugString());
                        return;
                    }
                }

                // update run step
                if (!run_service_->ModifyRunStep(modify_run_step_request)) {
                    LOG_ERROR("Failed to update run step object. modify_run_step_request={}", modify_run_step_request.ShortDebugString());
                    return;
                }
            }

            // update run object with status of `completed`
            if (!run_service_->ModifyRun(modify_run_request)) {
                LOG_ERROR("Failed to update run object. modify_run_request={}", modify_run_request.ShortDebugString());
                return;
            }

            LOG_INFO("OnAgentFinish Done, finish_message={}", finish_message.ShortDebugString());
        }


        // ReSharper disable once CppMemberFunctionMayBeConst
        std::optional<RunStepObject> RetrieveLastRunStep_(const RunObject& run_object) {
            ListRunStepsRequest list_run_steps_request;
            list_run_steps_request.set_order(desc);
            list_run_steps_request.set_run_id(run_object.id());
            list_run_steps_request.set_thread_id(run_object.thread_id());
            const auto list_run_steps_resp = run_service_->ListRunSteps(list_run_steps_request);
            if (list_run_steps_resp.data_size() > 0) {
                return *list_run_steps_resp.data().rbegin();
            }
            return std::nullopt;
        }


        bool LoadFunctionTools_(const RunObject& run_object, AgentState& state) const {
            std::vector<FunctionTool> function_tools;
            // 1. find tools on assistant
            GetAssistantRequest get_assistant_request;
            get_assistant_request.set_assistant_id(run_object.assistant_id());
            const auto assistant = assistant_service_->RetrieveAssistant(get_assistant_request);

            if (!assistant) {
                LOG_ERROR("Cannot find assistant object for run_object: {}", run_object.ShortDebugString());
                return false;
            }

            for (const auto& assistant_tool: assistant->tools()) {
                if (assistant_tool.type() == function) {
                    function_tools.push_back(assistant_tool.function());
                }
            }

            // 2. find tools on run object
            for (const auto& assistant_tool: run_object.tools()) {
                if (assistant_tool.type() == function) {
                    function_tools.push_back(assistant_tool.function());
                }
            }

            // 3. filter and transfrom to function tool schema
            const auto ret = std::ranges::unique(function_tools, [](const FunctionTool& a, const FunctionTool& b) {
                return a.name() == b.name();
            });
            function_tools.erase(ret.begin(), ret.end());
            LOG_DEBUG("Found {} function tools for run object: {}", function_tools.size(), run_object.ShortDebugString());
            for (auto& function_tool: function_tools) {
                state.mutable_function_tools()->Add()->CopyFrom(function_tool);
            }
            return true;
        }

        /**
         * this function is called when run object is in static state. It means no messages are generating and function tools are running.
         * @param run_object
         * @param state
         * @return true if state is loaded correctly
         */
        bool LoadAgentStateFromRun_(const RunObject& run_object, AgentState& state) const {
            if (run_object.status() == RunObject_RunObjectStatus_cancelling || run_object.status() == RunObject_RunObjectStatus_in_progress) {
                LOG_ERROR("Cannot handle run object that is canelling");
                return false;
            }

            // user last message as input
            const auto last_user_message = GetLatestUserMessageObject_(run_object.thread_id());
            if (!last_user_message) {
                LOG_ERROR("No user message found for run object: {}", run_object.ShortDebugString());
                return false;
            }
            if (last_user_message->content_size() <= 0) {
                LOG_ERROR("No valid message content found in user message: {}", last_user_message->ShortDebugString());
                return false;
            }
            auto* input_message = state.mutable_input()->mutable_chat()->add_messages();
            input_message->set_role("user");
            input_message->set_content(last_user_message->content(0).text().value());

            // find steps
            AgentStep* last_step = nullptr;
            AgentContinuation* last_continuation = nullptr;
            AgentPause* last_pause = nullptr;

            auto run_step_objects = ListAllSteps_(run_object.thread_id(), run_object.id());
            auto n = run_step_objects.size();
            LOG_DEBUG("Found {} steps for run object. run_object={}, run_steps={}",
                n,
                run_object.ShortDebugString(),
                StringUtils::JoinWith(run_step_objects | std::views::transform([](const RunStepObject& step) { return fmt::format("[run_step.id={}, run_step.status={}, run_step.type={}]", step.id(), RunStepObject_RunStepStatus_Name(step.status()), RunStepObject_RunStepType_Name(step.type())); }), " -> ")
            );
            for (int i=0;i<n;++i) {
                const auto& step = run_step_objects.at(i);
                LOG_DEBUG("step_id={}", step.id());
                if (step.type() == RunStepObject_RunStepType_tool_calls) {
                    if (step.step_details().tool_calls_size() <= 0) {
                        LOG_ERROR("should have tool calls in a run step");
                        return false;
                    }

                    // create continuation
                    last_step = state.mutable_previous_steps()->Add();
                    last_continuation = last_step->mutable_thought()->mutable_continuation();
                    // load custom data for this step
                    last_continuation->mutable_custom()->CopyFrom(step.step_details().custom());
                    auto* tool_call_request = last_continuation->mutable_tool_call_message();
                    tool_call_request->set_role("assistant");
                    for (const auto& tool_call: step.step_details().tool_calls()) {
                        // TODO support code-interpreter and file-search
                        if (tool_call.type() == function) {
                            auto* call_request = tool_call_request->add_tool_calls();
                            call_request->set_type(ToolCallObjectType::function);
                            call_request->set_id(tool_call.id());
                            call_request->mutable_function()->set_name(tool_call.function().name());
                            call_request->mutable_function()->set_arguments(tool_call.function().arguments());
                        }
                    }

                    if (i-1>=0) { // look backward for message
                        if (const auto& last_run_step = run_step_objects.at(i-1); last_run_step.type() == RunStepObject_RunStepType_message_creation) {
                            if (const auto message_obj = GetMessageObject_(run_object.thread_id(), last_run_step.step_details().message_creation().message_id())) { // update thought text line if last message is found
                                if (message_obj->content_size()>0) {
                                    tool_call_request->set_content(message_obj->content(0).text().value());
                                } else {
                                    LOG_WARN("Empty message found: {}", message_obj->ShortDebugString());
                                }

                            }
                        }
                    }

                    if (step.status() == RunStepObject_RunStepStatus_completed) {
                        // create observation
                        last_step = state.mutable_previous_steps()->Add();
                        auto* openai_observation = last_step->mutable_observation();
                        // load custom data for observation
                        openai_observation->mutable_custom()->CopyFrom(step.step_details().custom());
                        for (const auto& tool_call: step.step_details().tool_calls()) {
                            if (tool_call.type() == function) {
                                auto* tool_messsage = openai_observation->add_tool_messages();
                                tool_messsage->set_content(tool_call.function().output());
                                tool_messsage->set_role("tool");
                                tool_messsage->set_tool_call_id(tool_call.id());
                            }
                        }
                    }

                    if (step.status() == RunStepObject_RunStepStatus_in_progress) {
                        // because only run objects with status of `queued` and `requires_action` are allowed to be added to task scheduler
                        if (i != n-1) {
                            LOG_ERROR("it should be last step. i={}, step={}", i, step.ShortDebugString());
                            return false;
                        }
                        if (run_object.status() != RunObject_RunObjectStatus_queued) {
                            LOG_ERROR("run_object should be in status of queued");
                            return false;
                        }
                        // create pause
                        last_step = state.mutable_previous_steps()->Add();
                        last_pause = last_step->mutable_thought()->mutable_pause();
                        // load custom data for pause step
                        last_pause->mutable_custom()->CopyFrom(step.step_details().custom());
                        last_pause->mutable_tool_call_message()->CopyFrom(*tool_call_request);
                        // TODO support code-interpreter and file-search
                        // add tool messages for completed function tool calls, including those submitted by user
                        for (const auto& tool_call: step.step_details().tool_calls()) {
                            if (tool_call.type() == function && StringUtils::IsNotBlankString(tool_call.function().output())) {
                                auto* tool_messsage = last_pause->add_completed();
                                tool_messsage->set_content(tool_call.function().output());
                                tool_messsage->set_role("tool");
                                tool_messsage->set_tool_call_id(tool_call.id());
                            }
                        }
                        LOG_DEBUG("{}/{} completed tool calls in run step. thread_id={}, run_id={}, step_id={}", last_pause->completed_size(), last_pause->tool_call_message().tool_calls_size(), run_object.thread_id(), run_object.id(), step.id());
                    }

                    if (step.status() == RunStepObject_RunStepStatus_cancelled) {
                        // assert_true(i == n-1, "it should be last step");
                        // assert_true(run_object.status() == RunObject_RunObjectStatus_cancelled, "run object should be cancelled");
                        if (i != n-1) {
                            LOG_ERROR("it should be last step.");
                            return false;
                        }
                        if (run_object.status() != RunObject_RunObjectStatus_cancelled) {
                            LOG_ERROR("run_object should be in status of cancelled");
                            return false;
                        }
                        last_step = state.mutable_previous_steps()->Add();
                        auto* finish = last_step->mutable_thought()->mutable_finish();
                        finish->mutable_custom()->CopyFrom(step.step_details().custom());
                        finish->mutable_details()->PackFrom(step.last_error());
                        finish->set_is_cancelled(true);
                    }

                    if (step.status() == RunStepObject_RunStepStatus_expired) {
                        // assert_true(i == n-1, "it should be last step");
                        // assert_true(run_object.status() == RunObject_RunObjectStatus_expired, "run object should be expired");
                        if (i != n-1) {
                            LOG_ERROR("it should be last step.");
                            return false;
                        }
                        if (run_object.status() != RunObject_RunObjectStatus_expired) {
                            LOG_ERROR("run_object should be in status of expired");
                            return false;
                        }
                        last_step = state.mutable_previous_steps()->Add();
                        auto* finish = last_step->mutable_thought()->mutable_finish();
                        finish->mutable_custom()->CopyFrom(step.step_details().custom());
                        finish->mutable_details()->PackFrom(step.last_error());
                        finish->set_is_expired(true);
                    }

                    if (step.status() == RunStepObject_RunStepStatus_failed) {
                        // assert_true(i == n-1, "it should be last step");
                        // assert_true(run_object.status() == RunObject_RunObjectStatus_failed, "run object should be failed");
                        if (i != n-1) {
                            LOG_ERROR("it should be last step.");
                            return false;
                        }
                        if (run_object.status() != RunObject_RunObjectStatus_failed) {
                            LOG_ERROR("run_object should be in status of failed");
                            return false;
                        }
                        last_step = state.mutable_previous_steps()->Add();
                        auto* finish = last_step->mutable_thought()->mutable_finish();
                        finish->mutable_custom()->CopyFrom(step.step_details().custom());
                        finish->mutable_details()->PackFrom(step.last_error());
                        finish->set_is_failed(true);
                        if (step.has_last_error()) {
                            RunEarlyStopDetails run_early_stop_details;
                            run_early_stop_details.mutable_error()->CopyFrom(step.last_error());
                            finish->mutable_details()->PackFrom(run_early_stop_details);
                        }
                    }

                }

                if (step.type() == RunStepObject_RunStepType_message_creation) {
                    if (i == n-1 && run_object.status() == RunObject_RunObjectStatus_completed) { // last message
                        if (const auto message_obj = GetMessageObject_(run_object.thread_id(), step.step_details().message_creation().message_id())) {
                            last_step = state.mutable_previous_steps()->Add();
                            if (message_obj->content_size()>0) {
                                last_step->mutable_thought()->mutable_finish()->set_response(message_obj->content(0).text().value());
                            }
                        }
                    }
                    // other messages are fetched in `tool_calls` branch
                }


            }
            return true;
        }


        [[nodiscard]] std::optional<MessageObject> GetMessageObject_(const std::string& thread_id, const std::string& message_id) const {
            GetMessageRequest get_message_request;
            get_message_request.set_thread_id(thread_id);
            get_message_request.set_message_id(message_id);
            return message_service_->RetrieveMessage(get_message_request);
        }

        [[nodiscard]] std::vector<RunStepObject> ListAllSteps_(const std::string& thread_id, const std::string& run_id) const {
            std::vector<RunStepObject> run_step_objects;
            ListRunStepsRequest list_run_steps_request;
            list_run_steps_request.set_order(asc);
            list_run_steps_request.set_run_id(run_id);
            list_run_steps_request.set_thread_id(thread_id);
            ListRunStepsResponse list_run_steps_resp;
            list_run_steps_resp.set_has_more(true);
            do {
                if (list_run_steps_resp.data_size() > 0) {
                    list_run_steps_request.set_after(list_run_steps_resp.data().rbegin()->id());
                }
                list_run_steps_resp = run_service_->ListRunSteps(list_run_steps_request);
                run_step_objects.insert(run_step_objects.end(), list_run_steps_resp.data().begin(), list_run_steps_resp.data().end());
            } while (list_run_steps_resp.has_more());
            return run_step_objects;
        }

        [[nodiscard]] std::optional<MessageObject> GetLatestUserMessageObject_(const std::string& thread_id) const {
            ListMessagesRequest list_message_request;
            list_message_request.set_thread_id(thread_id);
            list_message_request.set_order(desc);
            ListMessageResponse list_message_response;
            list_message_response.set_has_more(true);
            do {
                if (list_message_response.data_size()>0) {
                    list_message_request.set_after(list_message_response.data().rbegin()->id());
                }
                list_message_response = message_service_->ListMessages(list_message_request);
                for (const auto& msg: list_message_response.data()) {
                    if (msg.role() == user) {
                        return msg;
                    }
                }
            } while (list_message_response.has_more());
            return std::nullopt;
        }





    };
}


#endif //RUNOBJECTHANDLER_HPP
