//
// Created by RobinQu on 2024/3/26.
//

#include <CLI/CLI.hpp>

#include <instinct/llm_object_factory.hpp>
#include <instinct/retriever_object_factory.hpp>
#include <instinct/document/recursive_character_text_splitter.hpp>
#include <instinct/chain/rag_chain.hpp>
#include <instinct/chat_model/ollama_chat.hpp>
#include <instinct/commons/ollama_commons.hpp>
#include <instinct/embedding_model/local_embedding_model.hpp>
#include <instinct/embedding_model/ollama_embedding.hpp>
#include <instinct/embedding_model/openai_embedding.hpp>
#include <instinct/ingestor/single_file_ingestor.hpp>
#include <instinct/ingestor/pdf_file_ingestor.hpp>
#include <instinct/ingestor/docx_file_ingestor.hpp>
#include <instinct/ingestor/parquet_file_ingestor.hpp>
#include <instinct/retrieval/chunked_multi_vector_retriever.hpp>
#include <instinct/retrieval/multi_query_retriever.hpp>
#include <instinct/retrieval/vector_store_retriever.hpp>
#include <instinct/store/duckdb/duckdb_doc_store.hpp>
#include <instinct/store/duckdb/duckdb_vector_store.hpp>
#include <instinct/endpoint/chat_completion/chat_completion_controller.hpp>
#include <instinct/ranker/local_ranking_model.hpp>
#include <instinct/retrieval/multi_path_retriever.hpp>
#include <instinct/retrieval/parent_child_retriever.hpp>
#include <instinct/retrieval/duckdb/duckdb_bm25_retriever.hpp>
#include <instinct/tools/assertions.hpp>


namespace instinct::examples::doc_agent {
    using namespace INSTINCT_RETRIEVAL_NS;
    using namespace INSTINCT_SERVER_NS;
    using namespace INSTINCT_LLM_NS;

    struct DocAgentRetrieverOptions: RetrieverOptions {
        int version = 2;
    };

    struct BuildCommandOptions {
        std::string filename;
        std::string file_type = "TXT";

        /**
         * see more at instinct::retrieval::CreateParquetIngestor
         */
        std::string parquet_mapping;

        std::string shared_db_file_path;
        DuckDBStoreOptions doc_store;
        size_t source_limit = 0;
        DuckDBStoreOptions vector_store;
        LLMProviderOptions chat_model_provider;
        LLMProviderOptions embedding_provider;
        bool force_rebuild = false;
        DocAgentRetrieverOptions retriever;
    };

    struct ServeCommandOptions {
        LLMProviderOptions chat_model_provider;
        LLMProviderOptions embedding_provider;
        std::string shared_db_file_path;
        DuckDBStoreOptions doc_store;
        DuckDBStoreOptions vector_store;
        ServerOptions server;
        DocAgentRetrieverOptions retriever;
    };

    class DocAgentApplicationContext final: public ManagedApplicationContext {
    public:
        IngestorPtr ingestor;
        DocStorePtr doc_store;
        EmbeddingsPtr embedding_model;
        ChatModelPtr chat_model;
        VectorStorePtr vector_store;
        StatefulRetrieverPtr chunked_vector_retriever;
        RetrieverPtr multipath_retriever;
        StatefulRetrieverPtr bm25_retriever;
        HttpLibServerPtr http_server;
    };

    static void PrintDatabaseSummary(const std::string& announcement, const DocStorePtr& doc_store, const VectorStorePtr& vector_store) {
        auto doc_count = doc_store->CountDocuments();
        auto vector_count = vector_store->CountDocuments();
        LOG_INFO("{}: DocStore {} docs, VectorStore {} embeddings", announcement, doc_count, vector_count);
    }


    class DocAgentApplicationContextFactory final: IApplicationContextFactory<DocAgentApplicationContext> {
        DocAgentApplicationContext instance_;
        std::once_flag init_flag_;
    public:
        explicit DocAgentApplicationContextFactory(const BuildCommandOptions& options) {
            instance_.ingestor = RetrieverObjectFactory::CreateIngestor(options.file_type, {
                .file_path = options.filename,
                .parquet_mapping = options.parquet_mapping,
                .fail_fast = true
            });
            instance_.embedding_model = LLMObjectFactory::CreateEmbeddingModel(options.embedding_provider);
            const auto db = std::make_shared<DuckDB>(options.shared_db_file_path);
            instance_.doc_store = CreateDuckDBDocStore(db, options.doc_store);
            instance_.vector_store = CreateDuckDBVectorStore(db, instance_.embedding_model, options.vector_store);
            instance_.chat_model = LLMObjectFactory::CreateChatModel(options.chat_model_provider);
            instance_.chunked_vector_retriever = CreateChunkedMultiVectorRetriever(options.retriever, instance_.doc_store, instance_.vector_store);
            if (options.retriever.version == 2) {
                // create BM25 retriever with vector_store which stores chunked documents instead of original docs
                instance_.bm25_retriever = CreateDuckDBBM25Retriever(instance_.vector_store, {.auto_build = false});
            }
        }

        explicit DocAgentApplicationContextFactory(const ServeCommandOptions& options) {
            instance_.embedding_model = LLMObjectFactory::CreateEmbeddingModel(options.embedding_provider);
            const auto db = std::make_shared<DuckDB>(options.shared_db_file_path);
            instance_.doc_store = CreateDuckDBDocStore(db, options.doc_store);
            instance_.vector_store = CreateDuckDBVectorStore(db, instance_.embedding_model, options.vector_store);
            instance_.chat_model = LLMObjectFactory::CreateChatModel(options.chat_model_provider);

            instance_.chunked_vector_retriever = CreateChunkedMultiVectorRetriever(options.retriever, instance_.doc_store, instance_.vector_store);

            RetrieverPtr stateless_retriever;
            if (options.retriever.version == 2) {
                instance_.bm25_retriever = CreateDuckDBBM25Retriever(instance_.vector_store, {.auto_build = false});
                instance_.multipath_retriever = CreateMultiPathRetriever(
                    CreateLocalRankingModel(BGE_M3_RERANKER),
                    instance_.chunked_vector_retriever,
                    // create a ParentChildRetriever that handles multi-hop search
                    CreateParentChildRetriever(instance_.doc_store, instance_.bm25_retriever)
                );
                stateless_retriever = instance_.multipath_retriever;
            }
            if (options.retriever.version == 1) {
                stateless_retriever = instance_.chunked_vector_retriever;
            }


            const auto question_prompt_template = CreatePlainPromptTemplate(R"(
Given the following conversation and a follow up question, rephrase the follow up question to be a standalone question, in its original language.
Chat History:
{chat_history}
Follow Up Input: {question}
Standalone question:)",
                                                                        {
                                                                            .input_keys = {"chat_history", "question"},
                                                                        });
            const auto answer_prompt_template = CreatePlainPromptTemplate(
                R"(Answer the question in a concise and clear way based only on the following context:
    {context}

    Question: {standalone_question}

    )", {.input_keys = {"context", "standalone_question"}});

            const auto question_fn = xn::steps::mapping({
                {
                    "standalone_question", xn::steps::mapping({
                                               // expecting `question` from input of `MappingData`
                                               {"question", xn::steps::selection("question")},
                                               {
                                                   "chat_history",
                                                   // expecting `chat_history` from input of `MappingData`.
                                                   xn::steps::selection("chat_history") | xn::steps::combine_chat_history()
                                               }
                                           }) | question_prompt_template | instance_.chat_model->AsModelFunction() |
                                           xn::steps::stringify_generation()
                },
                {
                    // expecting `question` from input of `MappingData`
                    "question", xn::steps::selection("question")
                }
            });

            const auto context_fn = xn::steps::mapping({
                {"context", xn::steps::selection("question") | stateless_retriever->AsContextRetrieverFunction({.top_k = 7})},
                {"standalone_question", xn::steps::selection("standalone_question")}
            });

            const auto rag_chain = question_fn | context_fn | answer_prompt_template | instance_.chat_model->AsModelFunction();

            instance_.http_server = CreateHttpLibServer(options.server);
            instance_.http_server->Use(CreateOpenAIChatCompletionController(rag_chain));
        }

        [[nodiscard]] const DocAgentApplicationContext & GetInstance() override {
            return instance_;
        }
    };

    using DocAgentApplicationContextFactoryPtr = std::shared_ptr<DocAgentApplicationContextFactory>;

    static std::vector<DocAgentApplicationContextFactoryPtr> CONTEXT_FACTORIES;

    static DocAgentApplicationContextFactoryPtr CreateContextFactory(const BuildCommandOptions& options) {
        const auto ptr = std::make_shared<DocAgentApplicationContextFactory>(options);
        CONTEXT_FACTORIES.push_back(ptr);
        return ptr;
    }

    static DocAgentApplicationContextFactoryPtr CreateContextFactory(const ServeCommandOptions& options) {
        const auto ptr = std::make_shared<DocAgentApplicationContextFactory>(options);
        CONTEXT_FACTORIES.push_back(ptr);
        return ptr;
    }

    static void GracefulShutdown(int signal) {
        LOG_INFO("GracefulShutdown due to signal {}", signal);
        GracefullyShutdownRunningHttpServers();
        for(const auto& f: CONTEXT_FACTORIES) {
            if (f) {
                f->GetInstance().Shutdown();
            }
        }
    }

    static void BuildCommand(const BuildCommandOptions& options) {
        const auto context_factory = CreateContextFactory(options);

        // force rebuild
        if (options.force_rebuild) {
            LOG_INFO("Force rebuild is enabled.");
            if (std::filesystem::remove(options.doc_store.db_file_path)) {
                LOG_INFO("DocStore file at {} is deleted", options.doc_store.db_file_path);
            }
            if (std::filesystem::remove(options.vector_store.db_file_path)) {
                LOG_INFO("VectorStore file at {} is deleted", options.vector_store.db_file_path);
            }
        } else {
            assert_true(!std::filesystem::exists(options.doc_store.db_file_path),
                        "DocStore file already exists. Abort to prevent data corruption.");
            assert_true(!std::filesystem::exists(options.vector_store.db_file_path),
                        "VectorStore file already exists. Abort to prevent data corruption.");
        }

        const auto& ctx = context_factory->GetInstance();
        ctx.chunked_vector_retriever->Ingest(ctx.ingestor->Load());
        if (const auto ptr = std::dynamic_pointer_cast<DuckDBBM25Retriever>(ctx.bm25_retriever)) {
            ptr->BuildIndex();
        }

        PrintDatabaseSummary("Database is built successfully", ctx.doc_store, ctx.vector_store);
    }

    static void ServeCommand(const ServeCommandOptions& options) {
        const auto context_factory = CreateContextFactory(options);
        const auto& ctx = context_factory->GetInstance();

        // register shutdown handler
        std::signal(SIGINT, GracefulShutdown);
        std::signal(SIGTERM, GracefulShutdown);

        // start context
        ctx.Startup();
        ctx.http_server->BindAndListen();
        ctx.Shutdown();
    }

    static void BuildDocStoreOptionGroup(CLI::Option_group* doc_store_ogroup,
                                         DuckDBStoreOptions& duck_db_options) {
        doc_store_ogroup
                ->add_option("--doc_table_name", duck_db_options.table_name, "Table name for documents")
                ->default_val("doc_table");
    }

    static void BuildVecstoreOptionGroup(CLI::Option_group* vec_store_ogroup,
                                         DuckDBStoreOptions& duck_db_options) {
        vec_store_ogroup
                ->add_option("--vector_table_dimension", duck_db_options.dimension, "Dimension of embedding vector.")
                ->check(CLI::Bound(1, 8192))
                ->required();
        vec_store_ogroup
                ->add_option("--vector_table_name", duck_db_options.table_name, "Table name for embedding table.")
                ->default_val("embedding_table");
    }

    static void BuildEmbeddingProviderOptionGroup(CLI::Option_group* llm_provider_ogroup,
                                            LLMProviderOptions& provider_options) {
        llm_provider_ogroup->description("Ollama, OpenAI API, or any OpenAI API compatible servers are supported.");
        llm_provider_ogroup->add_option("--embedding_model_provider", provider_options.provider,
                                        "Specify embedding model to use. ")
                ->transform(CLI::CheckedTransformer(model_provider_map, CLI::ignore_case));

        llm_provider_ogroup->add_option("--embedding_model_api_key", provider_options.api_key, "API key for commercial services like OpenAI. Leave blank for services without ACL.");
        llm_provider_ogroup->add_option("--embedding_model_host", provider_options.endpoint.host, "Host name for API endpoint, .e.g. 'api.openai.com' for OpenAI.");
        llm_provider_ogroup->add_option("--embedding_model_port", provider_options.endpoint.port, "Port number for API service.");
        llm_provider_ogroup->add_option("--embedding_model_protocol", provider_options.endpoint.protocol, "HTTP protocol for API service.")
                ->transform(CLI::CheckedTransformer(protocol_map, CLI::ignore_case));
        llm_provider_ogroup->add_option("--embedding_model_model_name", provider_options.model_name, "Specify name of the model to be used.");
    }

    static void BuildChatModelProviderOptionGroup(
        CLI::Option_group* llm_provider_ogroup,
        LLMProviderOptions& provider_options) {
        llm_provider_ogroup->description("Ollama, OpenAI API, or any OpenAI API compatible servers are supported. Defaults to a local running Ollama service using llama2:latest model.");
        llm_provider_ogroup
            ->add_option("--chat_model_provider", provider_options.provider, "Specify chat model to use for chat completion. ")
            ->transform(CLI::CheckedTransformer(model_provider_map, CLI::ignore_case));
        llm_provider_ogroup->add_option("--chat_model_api_key", provider_options.api_key, "API key for commercial services like OpenAI. Leave blank for services without ACL.");
        llm_provider_ogroup->add_option("--chat_model_host", provider_options.endpoint.host, "Host name for API endpoint, .e.g. 'api.openai.com' for OpenAI.");
        llm_provider_ogroup->add_option("--chat_model_port", provider_options.endpoint.port, "Port number for API service.");
        llm_provider_ogroup->add_option("--chat_model_protocol", provider_options.endpoint.protocol, "HTTP protocol for API service.")
                ->transform(CLI::CheckedTransformer(protocol_map, CLI::ignore_case));
        llm_provider_ogroup->add_option("--chat_model_model_name", provider_options.model_name, "Specify name of the model to be used.");
    }

    void BuildRetrieverOptions(CLI::Option_group* retriever_option_group, DocAgentRetrieverOptions& options) {
        // limited to one query_rewriter and one base_retriever
        retriever_option_group->add_option("--retriever_version", options.version, R"(Version 1: Use ChunkedMultiVectorRetriever only.
Version 2: Use ChunkedMultiVectorRetriever, and BM25 keyword-based Retriever together with a local reranker.
        )")
            ->default_val(2);
        const auto mv_ogroup = retriever_option_group->add_option_group("Options for ChunkedMultiVectorRetriever");
        mv_ogroup->add_option("--child_chunk_size", options.child_chunk_size, "chunk size for child document")
            ->default_val(200)
            ->check(CLI::Range{200, 10000});
        mv_ogroup->add_option("--parent_chunk_size", options.parent_chunk_size, "chunk size for parent document. Zero means disabling parent document splitter.")
            ->check(CLI::Range(0, 1000000))
            ->default_val(0);
    }
}


int main(int argc, char** argv) {
    using namespace CLI;
    using namespace instinct::examples::doc_agent;
    App app{
        "🤖 DocAgent: Chat with your documents locally with privacy. "
    };
    argv = app.ensure_utf8(argv);
    app.set_help_all_flag("--help-all", "Expand all help");

    // requires at least one sub-command
    app.require_subcommand();

    // llm_provider_options for both chat model and embedding model
    LLMProviderOptions chat_model_provider_options;
    BuildChatModelProviderOptionGroup(app.add_option_group("🧠 Provider for chat model"), chat_model_provider_options);

    LLMProviderOptions embedding_model_provider_options;
    BuildEmbeddingProviderOptionGroup(app.add_option_group("🧠 Provider for embedding model"), embedding_model_provider_options);

    // retriever options
    DocAgentRetrieverOptions retriever_options;
    BuildRetrieverOptions(app.add_option_group("🔍 Retriever", "Options for building retriever"), retriever_options);

    // db file path
    std::string shared_db_path;
    app.add_option("--db_path", shared_db_path, "DB file path for botch vetcor store and doc store.")->required();

    // vector store options
    DuckDBStoreOptions vector_store_options;
    BuildVecstoreOptionGroup(
    app.add_option_group("🔢 VectorStore"), vector_store_options);

    // doc store options
    DuckDBStoreOptions doc_store_options;
    BuildDocStoreOptionGroup(
    app.add_option_group("📖 DocStore"), doc_store_options);

    // build command
    BuildCommandOptions build_command_options;

    const auto build_command = app.add_subcommand(
        "build", "💼 Anaylize a single document and build database of learned context data. Proper values should be offered for Embedding model, Chat model, DocStore, VecStore and Retriever mentioned above.");
    build_command->add_flag("--force", build_command_options.force_rebuild,
                            "A flag to force rebuild of database, which means existing db files will be deleted. Use this option with caution!");
    auto ds_ogroup = build_command->add_option_group("Data source");
    ds_ogroup->add_option("-f,--file", build_command_options.filename, "Path to the document you want analyze")
            ->required();
    ds_ogroup
            ->add_option("-t,--type", build_command_options.file_type,
                         "File format of assigned document. Supported types are PDF,TXT,MD,DOCX,PARQUET")
            ->default_val("TXT")
            ->check(IsMember({"PDF", "DOCX", "MD", "TXT", "PARQUET"}, CLI::ignore_case));
    ds_ogroup->add_option("--parquet_mapping", build_command_options.parquet_mapping, "Mapping format for parquet columns. e.g. 1:t,2:m:parent_doc_id:int64,3:m:source:varchar.");
    ds_ogroup->add_option("--source_limit", build_command_options.source_limit, "Limit max entries from data source. It's supported only part of ingestors including PARQUET. Zero means no limit.")->default_val(0);

    build_command->final_callback([&]() {
        build_command_options.chat_model_provider = chat_model_provider_options;
        build_command_options.embedding_provider = embedding_model_provider_options;
        build_command_options.doc_store = doc_store_options;
        build_command_options.vector_store = vector_store_options;
        if(build_command_options.force_rebuild) {
            build_command_options.doc_store.create_or_replace_table = true;
            build_command_options.vector_store.create_or_replace_table = true;
        }
        build_command_options.retriever = retriever_options;
        build_command_options.shared_db_file_path = shared_db_path;
    });

    // serve command
    ServeCommandOptions serve_command_options;
    const auto serve_command = app.add_subcommand(
        "serve", "💃 Start a OpenAI API compatible server with database of learned context. Proper values should be offered for Chat model, DocStore, VecStore and Retriever mentioned above.");
    serve_command
            ->add_option("-p,--port", serve_command_options.server.port, "Port number which API server will listen")
            ->default_val("9090");
    serve_command->final_callback([&]() {
        serve_command_options.chat_model_provider = chat_model_provider_options;
        serve_command_options.embedding_provider = embedding_model_provider_options;
        serve_command_options.doc_store = doc_store_options;
        serve_command_options.vector_store = vector_store_options;
        serve_command_options.retriever = retriever_options;
        serve_command_options.shared_db_file_path = shared_db_path;
    });

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

    // trigger sub-command
    for (const auto* sub_command: app.get_subcommands()) {
        try {
            if (sub_command->get_name() == "build") {
                BuildCommand(build_command_options);
            }
            if (sub_command->get_name() == "serve") {
                ServeCommand(serve_command_options);
            }
        } catch (const std::runtime_error& err) {
            LOG_ERROR("Failed to execute sub-command {} due to exception. Possible reason: {}", sub_command->get_name(),
                      err.what());
            return 135;
        }
    }

    return 0;
}
