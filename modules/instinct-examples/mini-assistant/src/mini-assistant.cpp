//
// Created by RobinQu on 2024/4/19.
//
#include <CLI/CLI.hpp>


#include <instinct/../../../instinct-llm/include/instinct/llm_object_factory.hpp>
#include <instinct/agent/patterns/llm_compiler/llm_compiler_agent_executor.hpp>
#include <instinct/assistant/v2/endpoint/VectorStoreController.hpp>
#include <instinct/assistant/v2/endpoint/VectorStoreFileBatchController.hpp>
#include <instinct/assistant/v2/endpoint/VectorStoreFileController.hpp>
#include <instinct/assistant/v2/service/impl/VectorStoreServiceImpl.hpp>
#include <instinct/chat_model/ollama_chat.hpp>
#include <instinct/chat_model/openai_chat.hpp>
#include <instinct/commons/ollama_commons.hpp>
#include <instinct/database/db_utils.hpp>
#include <instinct/server/httplib/DefaultErrorController.hpp>
#include <instinct/store/duckdb/duckdb_vector_store_operator.hpp>
#include <instinct/server/httplib/HttpLibServer.hpp>
#include <instinct/assistant/v2/endpoint/AssistantController.hpp>
#include <instinct/assistant/v2/endpoint/FileController.hpp>
#include <instinct/assistant/v2/endpoint/MessageController.hpp>
#include <instinct/assistant/v2/endpoint/RunController.hpp>
#include <instinct/assistant/v2/endpoint/ThreadController.hpp>
#include <instinct/assistant/v2/tool/AssistantAPIApplicationContextFactory.hpp>
#include <instinct/assistant/v2/service/impl/AssistantServiceImpl.hpp>
#include <instinct/assistant/v2/service/impl/FileServiceImpl.hpp>
#include <instinct/assistant/v2/service/impl/MessageServiceImpl.hpp>
#include <instinct/assistant/v2/service/impl/RunServiceImpl.hpp>
#include <instinct/assistant/v2/service/impl/ThreadServiceImpl.hpp>




namespace instinct::examples::mini_assistant {
    using namespace INSTINCT_SERVER_NS;
    using namespace INSTINCT_LLM_NS;
    using namespace INSTINCT_ASSISTANT_NS::v2;


    struct ApplicationOptions {
        LLMProviderOptions embedding_model;
        LLMProviderOptions chat_model;
        ServerOptions server;
        ConnectionPoolOptions connection_pool;
        std::filesystem::path db_file_path;
        std::filesystem::path file_store_path;
        FileServiceOptions file_service;
        AgentExecutorOptions agent_executor;
        RetrieverOperatorOptions retriever_operator;
    };

    using MiniApplicationContext = ApplicationContext<duckdb::Connection, duckdb::unique_ptr<duckdb::MaterializedQueryResult>>;

    class MiniAssistantApplicationContextFactory final: public IApplicationContextFactory<MiniApplicationContext> {
        ApplicationOptions options_;
        std::once_flag init_flag_;
        MiniApplicationContext instance_{};
    public:
        explicit MiniAssistantApplicationContextFactory(ApplicationOptions application_options)
                : options_(std::move(application_options)) {}

        [[nodiscard]] const MiniApplicationContext& GetInstance() override  {
            std::call_once(init_flag_, [&] {
               Configure_(instance_);
            });
            return instance_;
        }

    private:
        void Configure_(MiniApplicationContext& context) {
            // configure database
            const auto duckdb = std::make_shared<DuckDB>(options_.db_file_path);
            context.connection_pool = CreateDuckDBConnectionPool(duckdb, options_.connection_pool);
            context.assistant_data_mapper = CreateDuckDBDataMapper<AssistantObject, std::string>(context.connection_pool);
            context.thread_data_mapper = CreateDuckDBDataMapper<ThreadObject, std::string>(context.connection_pool);
            context.message_data_mapper = CreateDuckDBDataMapper<MessageObject, std::string>(context.connection_pool);
            context.file_data_mapper = CreateDuckDBDataMapper<FileObject, std::string>(context.connection_pool);
            context.run_data_mapper = CreateDuckDBDataMapper<RunObject, std::string>(context.connection_pool);
            context.run_step_data_mapper = CreateDuckDBDataMapper<RunStepObject, std::string>(context.connection_pool);
            context.object_store = std::make_shared<FileSystemObjectStore>(options_.file_store_path);
            context.vector_store_data_mapper = std::make_shared<VectorStoreDataMapper>(CreateDuckDBDataMapper<VectorStoreObject, std::string>(context.connection_pool));
            context.vector_store_file_data_mapper = std::make_shared<VectorStoreFileDataMapper>(CreateDuckDBDataMapper<VectorStoreFileObject, std::string>(context.connection_pool));
            context.vector_store_file_batch_data_mapper = std::make_shared<VectorStoreFileBatchDataMapper>(CreateDuckDBDataMapper<VectorStoreFileBatchObject, std::string>(context.connection_pool));
            context.vector_store_metadata_data_mapper = std::make_shared<VectorStoreMetadataDataMapper>(CreateDuckDBDataMapper<VectorStoreInstanceMetadata, std::string>(context.connection_pool));
            context.db_migration = std::make_shared<assistant::DBMigration<duckdb::Connection, duckdb::unique_ptr<duckdb::MaterializedQueryResult>>>(options_.db_file_path, context.connection_pool);

            // configure task scheduler
            context.task_scheduler = CreateThreadPoolTaskScheduler(std::min(10u, std::thread::hardware_concurrency()));
            context.task_scheduler->Start();

            // configure services
            const auto thread_service = std::make_shared<ThreadServiceImpl>(context.thread_data_mapper, context.message_data_mapper, context.run_data_mapper, context.run_step_data_mapper);
            const auto message_service = std::make_shared<MessageServiceImpl>(context.message_data_mapper);
            const auto file_service = std::make_shared<FileServiceImpl>(context.file_data_mapper, context.object_store, options_.file_service);
            const auto run_service = std::make_shared<RunServiceImpl>(
                context.thread_data_mapper,
                context.run_data_mapper,
                context.run_step_data_mapper,
                context.message_data_mapper,
                context.task_scheduler
                );
            const auto assistant_service = std::make_shared<AssistantServiceImpl>(context.assistant_data_mapper);
            const auto embedding_model = LLMObjectFactory::CreateEmbeddingModel(options_.embedding_model);
            context.vector_store_operator = CreateDuckDBStoreOperator(
                duckdb,
                embedding_model,
                context.vector_store_metadata_data_mapper,
                CreateVectorStorePresetMetadataSchema()
                );
            context.retriever_operator = CreateSimpleRetrieverOperator(
                context.vector_store_operator,
                duckdb,
                {.table_name = "vs_document_table"},
                options_.retriever_operator
            );
            const auto vector_store_service = std::make_shared<VectorStoreServiceImpl>(context.vector_store_file_data_mapper, context.vector_store_data_mapper, context.vector_store_file_batch_data_mapper, context.task_scheduler, context.retriever_operator);
            context.assistant_facade = {
                .assistant = assistant_service,
                .file = file_service,
                .run = run_service,
                .thread = thread_service,
                .message = message_service,
                .vector_store = vector_store_service
            };

            // configure task handler for RunObject
            const auto chat_model = LLMObjectFactory::CreateChatModel(options_.chat_model);
            CitationAnnotatingChainPtr citation_annotating_chain = CreateCitationAnnotatingChain(chat_model);
            context.run_object_task_handler = std::make_shared<RunObjectTaskHandler>(
                run_service,
                message_service,
                assistant_service,
                context.retriever_operator,
                context.assistant_facade.vector_store,
                thread_service,
                citation_annotating_chain,
                options_.chat_model,
                options_.agent_executor
            );

            //  configure task handler for VectorStoreFileObject
            const auto summary_chain = CreateSummaryChain(chat_model);
            context.file_object_task_handler = std::make_shared<FileObjectTaskHandler>(
                context.retriever_operator,
                context.assistant_facade.vector_store,
                context.assistant_facade.file,
                summary_chain,
                FileObjectTaskHandlerOptions {.summary_input_max_size = 8*1024}
            );
            context.task_scheduler->RegisterHandler(context.run_object_task_handler);
            context.task_scheduler->RegisterHandler(context.file_object_task_handler);

            // background task for FileBatchObject
            context.file_batch_background_task = std::make_shared<FileBatchObjectBackgroundTask>(context.assistant_facade.vector_store, context.retriever_operator);

            // configure http server
            const auto http_server = CreateHttpLibServer(options_.server);
            const auto assistant_controller = std::make_shared<AssistantController>(context.assistant_facade);
            const auto file_controller = std::make_shared<FileController>(context.assistant_facade);
            const auto message_controller = std::make_shared<MessageController>(context.assistant_facade);
            const auto run_controller = std::make_shared<RunController>(context.assistant_facade);
            const auto thread_controller = std::make_shared<ThreadController>(context.assistant_facade);
            const auto error_controller = std::make_shared<DefaultErrorController>();
            const auto vector_store_controller = std::make_shared<VectorStoreController>(context.assistant_facade);
            const auto vector_store_file_controller = std::make_shared<VectorStoreFileController>(context.assistant_facade);
            const auto vector_store_file_batch_controller = std::make_shared<VectorStoreFileBatchController>(context.assistant_facade);
            http_server->Use(assistant_controller);
            http_server->Use(file_controller);
            http_server->Use(message_controller);
            http_server->Use(run_controller);
            http_server->Use(thread_controller);
            http_server->Use(error_controller);
            http_server->Use(vector_store_controller);
            http_server->Use(vector_store_file_controller);
            http_server->Use(vector_store_file_batch_controller);
            context.http_server = http_server;

            // add lifecycle objects
            context.Manage(context.file_batch_background_task);
            context.Manage(context.task_scheduler);
            context.Manage(context.db_migration);
        }
    };

    static std::shared_ptr<MiniAssistantApplicationContextFactory> CONTEXT_FACTORY;

    static void graceful_shutdown(int signal) {
        LOG_INFO("Begin shutdown due to signal {}", signal);
        instinct::server::GracefullyShutdownRunningHttpServers();
        CONTEXT_FACTORY->GetInstance().Shutdown();
        std::exit(0);
    }

    static void BuildAgentExecutorOptionsGroup(
        CLI::Option_group* agent_executor_option_group,
        LLMCompilerOptions& llm_compiler_option
    ) {
        agent_executor_option_group->description("Options for LLMCompiler-based agent executor");
        agent_executor_option_group->add_option("--max_replan", llm_compiler_option.max_replan, "Max count for replan")->default_val(6);
    }

}


int main(int argc, char** argv) {
    using namespace INSTINCT_SERVER_NS;
    using namespace INSTINCT_ASSISTANT_NS::v2;
    using namespace CLI;
    using namespace instinct::examples::mini_assistant;

    // register terminate handler to print dead message
    cpptrace::register_terminate_handler();

    App app{
            "🐬 mini-assistant - Local Assistant API at your service"
    };
    argv = app.ensure_utf8(argv);
    app.set_help_all_flag("--help-all", "Expand all help");

    ApplicationOptions application_options;
    app.add_option("-p,--port", application_options.server.port, "Port number which API server will listen")
            ->default_val("9091");

    app.add_option("--db_file_path", application_options.db_file_path, "Path for DuckDB database file.")->required();
    app.add_option("--file_store_path", application_options.file_store_path, "Path for root directory of local object store. Will be created if it doesn't exist yet.")
        ->required();

    {
        auto ogroup = app.add_option_group("chat_model", "Configuration for chat model");
        ogroup->add_option("--chat_model_provider", application_options.chat_model.provider,
                "Specify chat model to use for chat completion.")
            ->transform(CLI::CheckedTransformer(model_provider_map, CLI::ignore_case))
            ->default_val(ModelProvider::kOPENAI);
        ogroup->add_option("--chat_model_name", application_options.chat_model.model_name,
                                            "Specify chat model to use for chat completion. Default to gpt-3.5-turbo for OpenAI, llama3:8b for Ollama. Note that some model providers will ignore the passed model name and use the model currently loaded instead.");
        ogroup->add_option("--chat_model_api_key", application_options.chat_model.api_key, "API key for commercial services like OpenAI. Leave blank for services without ACL. API key is also retrieved from env variable named OPENAI_API_KEY.");
        ogroup->add_option("--chat_model_host", application_options.chat_model.endpoint.host, "Host name for API endpoint, .e.g. 'api.openai.com' for OpenAI.");
        ogroup->add_option("--chat_model_port", application_options.chat_model.endpoint.port, "Port number for API service.");
        ogroup->add_option("--chat_model_protocol", application_options.chat_model.endpoint.protocol, "HTTP protocol for API service.")
                ->transform(CLI::CheckedTransformer(protocol_map, CLI::ignore_case));
    }

    {
        auto ogroup = app.add_option_group("embedding_model", "Configuration for embedding model");
        ogroup
            ->add_option("--embedding_model_provider", application_options.embedding_model.provider,
                    "Specify model to use for embedding.")
            ->transform(CLI::CheckedTransformer(model_provider_map, CLI::ignore_case))
            ->default_val(ModelProvider::kOPENAI);
        ogroup->add_option("--embedding_model_name", application_options.embedding_model.model_name,
                                            "Specify model to use for embedding . Default to text-embedding-3-small for OpenAI, all-minilm:latest for Ollama. Note that some model providers will ignore the passed model name and use the model currently loaded instead.");
        ogroup->add_option("--embedding_model_dim", application_options.embedding_model.dim, "Dimension of given embedding model.")
            ->check(PositiveNumber);
        ogroup->add_option("--embedding_model_api_key", application_options.embedding_model.api_key, "API key for commercial services like OpenAI. Leave blank for services without ACL. API key is also retrieved from env variable named OPENAI_API_KEY.");
        ogroup->add_option("--embedding_model_host", application_options.embedding_model.endpoint.host, "Host name for API endpoint, .e.g. 'api.openai.com' for OpenAI.");
        ogroup->add_option("--embedding_model_port", application_options.embedding_model.endpoint.port, "Port number for API service.");
        ogroup->add_option("--embedding_model_protocol", application_options.embedding_model.endpoint.protocol, "HTTP protocol for API service.")
                ->transform(CLI::CheckedTransformer(protocol_map, CLI::ignore_case));
    }

    app.add_option("--agent_executor_type", application_options.agent_executor.agent_executor_name, "Specify agent executor type. `llm_compiler` enables parallel function calling with opensourced models like mistral series and llama series, while `openai_tool` relies on official OpenAI function calling capability to direct agent workflow.")
        ->check(CLI::IsMember({"llm_compiler", "openai_tool"}, CLI::ignore_case))
        ->default_val("llm_compiler");
    BuildAgentExecutorOptionsGroup(app.add_option_group("Options for LLMCompilerAgentExecutor"), application_options.agent_executor.llm_compiler);

    // log level
    bool enable_verbose_log;
    app.add_flag("-v,--verbose", enable_verbose_log, "A flag to enable verbose log");

    // parse input
    CLI11_PARSE(app, argc, argv);

    // setup logging
    if (enable_verbose_log) {
        fmtlog::setLogLevel(fmtlog::DBG);
        LOG_INFO("Verbose logging is enabled.");
    } else {
        LOG_INFO("Logging level is default to INFO");
    }
    fmtlog::startPollingThread();

    // register shutdown handler
    std::signal(SIGINT, graceful_shutdown);
    std::signal(SIGTERM, graceful_shutdown);

    // build context
    CONTEXT_FACTORY = std::make_shared<MiniAssistantApplicationContextFactory>(application_options);

    const auto& ctx = CONTEXT_FACTORY->GetInstance();

    // start application context
    ctx.Startup();

    // start server
    ctx.http_server->BindAndListen();

    // cleanup
    ctx.Shutdown();
}