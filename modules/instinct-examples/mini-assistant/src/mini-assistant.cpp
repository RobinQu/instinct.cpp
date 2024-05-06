//
// Created by RobinQu on 2024/4/19.
//
#include <CLI/CLI.hpp>
#include <cmrc/cmrc.hpp>

#include "chat_model/OpenAIChat.hpp"
#include "database/DBUtils.hpp"
#include "server/httplib/DefaultErrorController.hpp"
#include "toolkit/LocalToolkit.hpp"

CMRC_DECLARE(instinct::assistant);

#include "server/httplib/HttpLibServer.hpp"
#include "assistant/v2/endpoint/AssistantController.hpp"
#include "assistant/v2/endpoint/FileController.hpp"
#include "assistant/v2/endpoint/MessageController.hpp"
#include "assistant/v2/endpoint/RunController.hpp"
#include "assistant/v2/endpoint/ThreadController.hpp"
#include "assistant/v2/tool/IApplicationContextFactory.hpp"
#include "assistant/v2/service/impl/AssistantServiceImpl.hpp"
#include "assistant/v2/service/impl/FileServiceImpl.hpp"
#include "assistant/v2/service/impl/MessageServiceImpl.hpp"
#include "assistant/v2/service/impl/RunServiceImpl.hpp"
#include "assistant/v2/service/impl/ThreadServiceImpl.hpp"




namespace instinct::examples::mini_assistant {
    using namespace INSTINCT_SERVER_NS;
    using namespace INSTINCT_LLM_NS;
    using namespace INSTINCT_ASSISTANT_NS::v2;

    struct ApplicationOptions {
        ServerOptions server;
        ConnectionPoolOptions connection_pool;
        std::filesystem::path db_file_path;
        std::filesystem::path file_store_path;
        FileServiceOptions file_service;
    };

    class MiniAssistantApplicationContextFactory final: public IApplicationContextFactory<duckdb::Connection, duckdb::unique_ptr<duckdb::MaterializedQueryResult>> {
        ApplicationOptions options_;
    public:
        explicit MiniAssistantApplicationContextFactory(ApplicationOptions applicationOptions)
                : options_(std::move(applicationOptions)) {}

        ApplicationContext GetInstance() override {
            static ApplicationContext context;
            // configure database
            const auto duck_db_ = std::make_shared<DuckDB>(options_.db_file_path);
            context.connection_pool = CreateDuckDBConnectionPool(duck_db_, options_.connection_pool);
            context.assistant_data_mapper = CreateDuckDBDataMapper<AssistantObject, std::string>(context.connection_pool);
            context.thread_data_mapper = CreateDuckDBDataMapper<ThreadObject, std::string>(context.connection_pool);
            context.message_data_mapper = CreateDuckDBDataMapper<MessageObject, std::string>(context.connection_pool);
            context.file_data_mapper = CreateDuckDBDataMapper<FileObject, std::string>(context.connection_pool);
            context.run_data_mapper = CreateDuckDBDataMapper<RunObject, std::string>(context.connection_pool);
            context.run_step_data_mapper = CreateDuckDBDataMapper<RunStepObject, std::string>(context.connection_pool);
            context.object_store = std::make_shared<FileSystemObjectStore>(options_.file_store_path);

            // configure task scheduler
            context.task_scheduler = CreateThreadPoolTaskScheduler();
            context.task_scheduler->Start();

            // configure services
            const auto thread_service = std::make_shared<ThreadServiceImpl>(context.thread_data_mapper, context.message_data_mapper);
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
            context.assistant_facade = {
                .assistant = assistant_service,
                .file = file_service,
                .run = run_service,
                .thread = thread_service,
                .message = message_service,
                .vector_store = nullptr
            };

            // configure task handler for run objects
            auto builtin_toolkit = CreateLocalToolkit({});
            context.run_object_task_handler = std::make_shared<OpenAIToolAgentRunObjectTaskHandler>(
                run_service,
                message_service,
                assistant_service,
                [](const RunObject&) { return CreateOpenAIChatModel(); },
                builtin_toolkit
                );
            context.task_scheduler->RegisterHandler(context.run_object_task_handler);

            // configure http server
            const auto http_server = std::make_shared<HttpLibServer>(options_.server);
            const auto assistant_controller = std::make_shared<AssistantController>(context.assistant_facade);
            const auto file_controller = std::make_shared<FileController>(context.assistant_facade);
            const auto message_controller = std::make_shared<MessageController>(context.assistant_facade);
            const auto run_controller = std::make_shared<RunController>(context.assistant_facade);
            const auto thread_controller = std::make_shared<ThreadController>(context.assistant_facade);
            const auto error_controller = std::make_shared<DefaultErrorController>();
            http_server->Use(assistant_controller);
            http_server->Use(file_controller);
            http_server->Use(message_controller);
            http_server->Use(run_controller);
            http_server->Use(thread_controller);
            http_server->Use(error_controller);
            context.http_server = http_server;
            return context;
        }
    };

}

static void graceful_shutdown(int signal) {
    LOG_INFO("Begin shutdown due to signal {}", signal);
    instinct::server::GracefullyShutdownRunningHttpServers();
    instinct::data::GracefullyShutdownThreadPoolTaskSchedulers();
    std::exit(0);
}

int main(int argc, char** argv) {
    using namespace INSTINCT_SERVER_NS;
    using namespace INSTINCT_ASSISTANT_NS::v2;
    using namespace CLI;
    using namespace instinct::examples::mini_assistant;

    App app{
            "mini-assistant - Local Assistant API at your service"
    };
    argv = app.ensure_utf8(argv);
    app.set_help_all_flag("--help-all", "Expand all help");

    ApplicationOptions application_options;
    app.add_option("-p,--port", application_options.server.port, "Port number which API server will listen")
            ->default_val("9091");
    app.add_option("--db_file_path", application_options.db_file_path, "Path for DuckDB database file.");
    app.add_option("--file_store_path", application_options.file_store_path, "Path for root directory of local object store.");

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

    // build context and start http server
    MiniAssistantApplicationContextFactory factory {application_options};
    const auto context = factory.GetInstance();

    // db migration
    auto fs = cmrc::instinct::assistant::get_filesystem();
    auto sql_file = fs.open("db_migration/001/up.sql");
    const auto sql_line = std::string {sql_file.begin(), sql_file.end()};
    LOG_DEBUG("initialize database at {} with sql:\n {}", application_options.db_file_path, sql_line);
    DBUtils::ExecuteSQL(sql_line, context.connection_pool);

    // start server
    context.http_server->StartAndWait();
}