//
// Created by RobinQu on 2024/4/19.
//

#include <CLI/CLI.hpp>


#include "server/httplib/HttpLibServer.hpp"
#include "assistant/v2/endpoint/AssistantController.hpp"
#include "assistant/v2/tool/IApplicaitonContextFactory.hpp"
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
    };

    class MiniAssistantApplicationContextFactory: public IApplicationContextFactory<duckdb::Connection, duckdb::unique_ptr<duckdb::MaterializedQueryResult>> {
        ApplicationOptions options_;
    public:
        explicit MiniAssistantApplicationContextFactory(ApplicationOptions applicationOptions)
                : options_(std::move(applicationOptions)) {}

        ApplicationContext GetInstance() override {
            ApplicationContext context;
            DuckDBPtr duck_db_ = std::make_shared<DuckDB>(options_.db_file_path);
            context.connection_pool = CreateDuckDBConnectionPool(duck_db_, options_.connection_pool);
            context.assistant_data_mapper = CreateDuckDBDataMapper<AssistantObject, std::string>(context.connection_pool);
            context.thread_data_mapper = CreateDuckDBDataMapper<ThreadObject, std::string>(context.connection_pool);
            context.message_data_mapper = CreateDuckDBDataMapper<MessageObject, std::string>(context.connection_pool);
            context.file_data_mapper = CreateDuckDBDataMapper<FileObject, std::string>(context.connection_pool);
            context.run_data_mapper = CreateDuckDBDataMapper<RunObject, std::string>(context.connection_pool);
            context.run_step_data_mapper = CreateDuckDBDataMapper<RunStepObject, std::string>(context.connection_pool);
            context.task_scheduler = CreateThreadPoolTaskScheduler();
            context.object_store = std::make_shared<FileSystemObjectStore>(options_.file_store_path);


            const auto thread_service = std::make_shared<ThreadServiceImpl>(context.thread_data_mapper, context.message_data_mapper);
            const auto message_service = std::make_shared<MessageServiceImpl>(context.message_data_mapper);

            
            return context;
        }
    };

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

    // requires at least one sub-command
    app.require_subcommand();

    ApplicationOptions application_options;
    app.add_option("-p,--port", application_options.server.port, "Port number which API server will listen")
            ->default_val("9091");
    app.add_option("--db_file_path", application_options.db_file_path, "Path for DuckDB database file.");
    app.add_option("--file_store_path", application_options.file_store_path, "Path for root directory of local object store.");

    MiniAssistantApplicationContextFactory factory {application_options};
    const auto context = factory.GetInstance();
    context.http_server->StartAndWait();
}