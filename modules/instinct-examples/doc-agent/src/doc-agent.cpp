//
// Created by RobinQu on 2024/3/26.
//

#include <CLI/CLI.hpp>

#include "document/RecursiveCharacterTextSplitter.hpp"
#include "chain/RAGChain.hpp"
#include "chat_model/OllamaChat.hpp"
#include "commons/OllamaCommons.hpp"
#include "embedding_model/OllamaEmbedding.hpp"
#include "embedding_model/OpenAIEmbedding.hpp"
#include "ingestor/SingleFileIngestor.hpp"
#include "ingestor/PDFFileIngestor.hpp"
#include "ingestor/DOCXFileIngestor.hpp"
#include "ingestor/ParquetFileIngestor.hpp"
#include "retrieval/ChunkedMultiVectorRetriever.hpp"
#include "retrieval/MultiQueryRetriever.hpp"
#include "retrieval/VectorStoreRetriever.hpp"
#include "store/duckdb/DuckDBDocStore.hpp"
#include "store/duckdb/DuckDBVectorStore.hpp"
#include "tools/http/OpenAICompatibleAPIServer.hpp"
#include "tools/Assertions.hpp"


namespace insintct::exmaples::doc_agent {
    using namespace INSTINCT_RETRIEVAL_NS;
    using namespace INSTINCT_SERVER_NS;
    using namespace INSTINCT_LLM_NS;


    struct LLMProviderOptions {
        std::string provider_name = "ollama";
        // OpenAIConfiguration openai;
        // OllamaConfiguration ollama;

        std::string host;
        int port = 0;
        HttpProtocol protocol = kHTTPS;
        std::string model_name;
        std::string api_key;
    };


    struct RetrieverOptions {
        bool plain_vector_retriever = false;
        bool summary_guided_retriever = false;
        bool hypothectical_queries_guided_retriever = false;
        bool chunked_multi_vector_retriever = false;
        bool multi_query_retriever = false;
        bool auto_retriever = false;

        int parent_chunk_size = 1000;
        int child_chunk_size = 200;
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
        DuckDBStoreOptions vector_store;
        LLMProviderOptions chat_model_provider;
        LLMProviderOptions embedding_provider;
        bool force_rebuild = false;
        RetrieverOptions retriever;
    };

    struct ServeCommandOptions {
        LLMProviderOptions chat_model_provider;
        LLMProviderOptions embedding_provider;
        // TODO DBStore refactoring to make it possible to share duckdb instantce.
        std::string shared_db_file_path;
        DuckDBStoreOptions doc_store;
        DuckDBStoreOptions vector_store;
        ServerOptions server;
        RetrieverOptions retriever;
    };

    static EmbeddingsPtr CreateEmbeddingModel(const LLMProviderOptions& options) {
        if (options.provider_name == "ollama") {
            return CreateOllamaEmbedding({
                .endpoint = {.protocol = options.protocol, .host = options.host, .port = options.port},
                .model_name = options.model_name
            });
        }
        if (options.provider_name == "openai") {
            return CreateOpenAIEmbeddingModel({
                .endpoint = {.protocol = options.protocol, .host = options.host, .port = options.port},
                .model_name = options.model_name
            });
        }
        return nullptr;
    }

    static ChatModelPtr CreateChatModel(const LLMProviderOptions& options) {
        if (options.provider_name == "ollama") {
            return CreateOllamaChatModel({
                .endpoint = {.protocol = options.protocol, .host = options.host, .port = options.port},
                .model_name = options.model_name
            });
        }
        if (options.provider_name == "openai") {
            return CreateOpenAIChatModel({
                .endpoint = {.protocol = options.protocol, .host = options.host, .port = options.port},
                .model_name = options.model_name
            });
        }
        return nullptr;
    }

    static void PrintDatabaseSummary(const std::string& announcment, const DocStorePtr& doc_store, const VectorStorePtr& vectore_store) {
        auto doc_count = doc_store->CountDocuments();
        auto vector_count = vectore_store->CountDocuments();
        LOG_INFO("{}: DocStore {} docs, VectorStore {} embeddings", announcment, doc_count, vector_count);
    }

    static StatefulRetrieverPtr CreateBaseRetriever(
        const RetrieverOptions& retriever_options,
        const DocStorePtr& doc_store,
        const VectorStorePtr& vector_store,
        const ChatModelPtr& chat_model
    ) {
        if (retriever_options.plain_vector_retriever) {
            LOG_INFO("CreateVectorStoreRetriever");
            return CreateVectorStoreRetriever(vector_store);
        }

        if(retriever_options.chunked_multi_vector_retriever) {
            LOG_INFO("CreateChunkedMultiVectorRetriever");
            const auto child_spliter = CreateRecursiveCharacterTextSplitter({.chunk_size = retriever_options.child_chunk_size});
            if (retriever_options.parent_chunk_size > 0) {
                assert_true(retriever_options.parent_chunk_size > retriever_options.child_chunk_size, "parent_chunk_size should be larger than child_chunk_size");
                const auto parent_splitter = CreateRecursiveCharacterTextSplitter({.chunk_size = retriever_options.parent_chunk_size});
                return CreateChunkedMultiVectorRetriever(
                    doc_store,
                    vector_store,
                    child_spliter,
                    parent_splitter
                );
            }
            return CreateChunkedMultiVectorRetriever(
                doc_store,
                vector_store,
                child_spliter
            );

        }

        if(retriever_options.hypothectical_queries_guided_retriever) {
            LOG_INFO("CreateHypotheticalQueriesGuidedRetriever");
            return CreateHypotheticalQueriesGuidedRetriever(
                chat_model,
                doc_store,
                vector_store
            );
        }

        if (retriever_options.summary_guided_retriever) {
            LOG_INFO("CreateSummaryGuidedRetriever");
            return CreateSummaryGuidedRetriever(
                chat_model,
                doc_store,
                vector_store
            );
        }
        return nullptr;
    }

    static RetrieverPtr CreateAdaptorRetriever(
        const RetrieverOptions& retriever_options,
        const StatefulRetrieverPtr& base_retriever,
        const ChatModelPtr& chat_model
    ) {
        if (retriever_options.multi_query_retriever) {
            LOG_INFO("CreateMultiQueryRetriever");
            return CreateMultiQueryRetriever(base_retriever, chat_model);
        }

        // if no adaptor retriever is required, return base retirever instead
        return base_retriever;
    }

    static void BuildCommand(const BuildCommandOptions& options) {
        // create ingestor
        IngestorPtr ingestor;
        if (options.file_type == "PDF") {
            ingestor = CreatePDFFileIngestor(options.filename);
        } else if (options.file_type == "DOCX") {
            ingestor = CreateDOCXFileIngestor(options.filename);
        } else if (options.file_type == "PARQUET") {
            assert_true(!options.parquet_mapping.empty(), "should provide mapping format for parquet file");
            ingestor = CreateParquetIngestor(options.filename, options.parquet_mapping);
        } else {
            ingestor = CreatePlainTextFileIngestor(options.filename);
        }

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

        // shared duckdb
        const auto db = std::make_shared<DuckDB>(options.shared_db_file_path);

        // doc store
        const auto doc_store = CreateDuckDBDocStore(db, options.doc_store);

        // embedding model
        EmbeddingsPtr embedding_model = CreateEmbeddingModel(options.embedding_provider);
        assert_true(embedding_model, "should have assigned correct embedding model");

        // chat model
        const auto chat_model = CreateChatModel(options.chat_model_provider);
        assert_true(chat_model, "should have assigned correct chat model");

        // vector model
        const auto vectore_store = CreateDuckDBVectorStore(db, embedding_model, options.vector_store);

        // base retriever is enough for data ingestion
        const auto retriever = CreateBaseRetriever(
            options.retriever,
            doc_store,
            vectore_store,
            chat_model
        );
        assert_true(retriever, "Should have assigned correct retriever");

        // ingest data
        retriever->Ingest(ingestor->Load());

        PrintDatabaseSummary("Database is built successfully", doc_store, vectore_store);
    }

    static void ServeCommand(const ServeCommandOptions& options) {
        EmbeddingsPtr embedding_model = CreateEmbeddingModel(options.embedding_provider);
        assert_true(embedding_model, "should have assigned correct embedding model");

        const auto chat_model = CreateChatModel(options.chat_model_provider);
        assert_true(chat_model, "should have assigned correct chat model");

        const auto db = std::make_shared<DuckDB>(options.shared_db_file_path);

        const auto doc_store = CreateDuckDBDocStore(db, options.doc_store);
        const auto vector_store = CreateDuckDBVectorStore(db, embedding_model, options.vector_store);
        PrintDatabaseSummary("VectorStore and DocStore loaded: ", doc_store, vector_store);

        const auto base_retriever = CreateBaseRetriever(options.retriever, doc_store, vector_store, chat_model);
        assert_true(base_retriever, "Should have assigned correct retriever");
        const auto retriever = CreateAdaptorRetriever(options.retriever, base_retriever, chat_model);

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
            R"(Answer the question based only on the following context:
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
                                       }) | question_prompt_template | chat_model->AsModelFunction() |
                                       xn::steps::stringify_generation()
            },
            {
                // expecting `question` from input of `MappingData`
                "question", xn::steps::selection("question")
            }
        });

        const auto context_fn = xn::steps::mapping({
            {"context", xn::steps::selection("question") | retriever->AsContextRetrieverFunction()},
            {"standalone_question", xn::steps::selection("standalone_question")}
        });

        const auto rag_chain = question_fn | context_fn | answer_prompt_template | chat_model->AsModelFunction();

        const auto server_chain = CreateOpenAIServerChain(rag_chain);

        OpenAICompatibleAPIServer server(server_chain, options.server);
        server.StartAndWait();
    }


    static void BuildDocstoreOptionGroup(CLI::Option_group* doc_store_ogroup,
                                         DuckDBStoreOptions& duck_db_options) {
        // doc_store_ogroup
        //         ->add_option("--doc_db_path", duck_db_options.db_file_path,
        //                      "File path to database file, which will be created if given file doesn't exist.");
        doc_store_ogroup
                ->add_option("--doc_table_name", duck_db_options.table_name, "Table name for documents")
                ->default_val("doc_table");
    }

    static void BuildVecstoreOptionGroup(CLI::Option_group* vec_store_ogroup,
                                         DuckDBStoreOptions& duck_db_options) {
        // vec_store_ogroup
        //         ->add_option("--vector_db_path", duck_db_options.db_file_path,
        //                      "File path to database file, which will be created if given file doesn't exist.");
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

        llm_provider_ogroup->description("Ollama, OpenAI API, or any OpenAI API compatible servers are supported. Defaults to a local running Ollama service using llama2:latest model.");
        llm_provider_ogroup->add_option("--embedding_model_provider", provider_options.provider_name,
                                        "Specify embedding model to use. ")
                ->check(CLI::IsMember({"ollama", "openai"}, CLI::ignore_case))
                ->default_val("ollama");

        const std::map<std::string, HttpProtocol> protocol_map{
            {"http", kHTTP},
            {"https", kHTTPS}
        };
        llm_provider_ogroup->add_option("--embedding_model_api_key", provider_options.api_key, "API key for comercial services like OpenAI. Leave blank for services without ACL.");
        llm_provider_ogroup->add_option("--embedding_model_host", provider_options.host, "Host name for API endpoint, .e.g. 'api.openai.com' for OpenAI.")
                ->default_val(OLLAMA_ENDPOINT.host);
        llm_provider_ogroup->add_option("--embedding_model_port", provider_options.port, "Port number for API service.")
                ->default_val(OLLAMA_ENDPOINT.port);
        llm_provider_ogroup->add_option("--embedding_model_protocol", provider_options.protocol, "HTTP protocol for API service.")
                ->transform(CLI::CheckedTransformer(protocol_map, CLI::ignore_case))
                ->default_val(OLLAMA_ENDPOINT.protocol);
        llm_provider_ogroup->add_option("--embedding_model_model_name", provider_options.model_name, "Specify name of the model to be used.")
                ->default_val(OLLAMA_DEFUALT_MODEL_NAME);
    }

    static void BuildChatModelProviderOptionGroup(
        CLI::Option_group* llm_provider_ogroup,
        LLMProviderOptions& provider_options) {
        llm_provider_ogroup->description("Ollama, OpenAI API, or any OpenAI API compatible servers are supported. Defaults to a local running Ollama service using llama2:latest model.");
        llm_provider_ogroup->add_option("--chat_model_provider", provider_options.provider_name,
                                        "Specify chat model to use for chat completion. ")
                ->check(CLI::IsMember({"ollama", "openai"}, CLI::ignore_case))
                ->default_val("ollama");

        const std::map<std::string, HttpProtocol> protocol_map{
            {"http", kHTTP},
            {"https", kHTTPS}
        };

        llm_provider_ogroup->add_option("--chat_model_api_key", provider_options.api_key, "API key for comercial services like OpenAI. Leave blank for services without ACL.");
        llm_provider_ogroup->add_option("--chat_model_host", provider_options.host, "Host name for API endpoint, .e.g. 'api.openai.com' for OpenAI.")
                ->default_val(OLLAMA_ENDPOINT.host);
        llm_provider_ogroup->add_option("--chat_model_port", provider_options.port, "Port number for API service.")
                ->default_val(OLLAMA_ENDPOINT.port);
        llm_provider_ogroup->add_option("--chat_model_protocol", provider_options.protocol, "HTTP protocol for API service.")
                ->transform(CLI::CheckedTransformer(protocol_map, CLI::ignore_case))
                ->default_val(OLLAMA_ENDPOINT.protocol);
        llm_provider_ogroup->add_option("--chat_model_model_name", provider_options.model_name, "Specify name of the model to be used.")
                ->default_val(OLLAMA_DEFUALT_MODEL_NAME);
    }

    void BuildRetrieverOptions(CLI::Option_group* retriever_option_group, RetrieverOptions& options) {
        // limited to one query_rewriter and one base_retriever
        const auto base_retriever_ogroup = retriever_option_group->add_option_group("base_retriever", "A must-to-have base retriever that handles original documents.");
        base_retriever_ogroup->add_flag("--plain_vector_retriever", options.plain_vector_retriever, "Enable VectorStoreRetiever.");
        base_retriever_ogroup
            ->add_flag("--parent_child_retriever", options.chunked_multi_vector_retriever, "Enable ChunkedMultiVectorRetriever.");
        base_retriever_ogroup->add_flag("--summary_guided_retriever", options.summary_guided_retriever, "Enable MultiVectorGuidance with summary guidance.");
        base_retriever_ogroup->add_flag("--hypothetical_quries_guided_retriever", options.hypothectical_queries_guided_retriever, "Enable MultiVectorGuidance with hypothetical queries.");
        // one base retriever is required
        base_retriever_ogroup->require_option(1, 1);

        const auto mv_ogroup = retriever_option_group->add_option_group("Options for ChunkedMultiVectorRetriever");
        mv_ogroup->add_option("--child_chunk_size", options.child_chunk_size, "chunk size for child document")
            ->default_val(200)
            ->check(CLI::Range{200, 10000});
        mv_ogroup->add_option("--parent_chunk_size", options.parent_chunk_size, "chunk size for parent document. Zero means disabling parent document splitter.")
            ->check(CLI::Range(0, 1000000))
            ->default_val(0);

        const auto query_rerwier_ogorup = retriever_option_group->add_option_group("query_rewriter", "Adaptor retrievers that rewrites original query.");
        query_rerwier_ogorup->add_flag("--multi_query_retriever",  options.multi_query_retriever, "Enable MultiQueryRetriever.");
        // at most one query_retriever is specified
        query_rerwier_ogorup->require_option(-1);
    }
}


int main(int argc, char** argv) {
    using namespace CLI;
    using namespace insintct::exmaples::doc_agent;
    App app{
        "🤖 DocAgent: Chat with your documents locally with privacy. "
    };
    argv = app.ensure_utf8(argv);
    app.set_help_all_flag("--help-all", "Expand all help");

    // requires at least one sub-command
    app.require_subcommand();

    // llm_provider_options for both chat model and embedding model
    LLMProviderOptions chat_model_provider_options;
    BuildChatModelProviderOptionGroup(app.add_option_group("🧠 Provider for embedding model"), chat_model_provider_options);

    LLMProviderOptions embedding_model_provider_options;
    BuildEmbeddingProviderOptionGroup(app.add_option_group("🧠 Provider for chat model"), embedding_model_provider_options);

    // retriever options
    RetrieverOptions retriever_options;
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
    BuildDocstoreOptionGroup(
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
