//
// Created by RobinQu on 2024/3/26.
//

#include <CLI/CLI.hpp>
#include <document/RecursiveCharacterTextSplitter.hpp>

#include "chain/RAGChain.hpp"
#include "chat_model/OllamaChat.hpp"
#include "commons/OllamaCommons.hpp"
#include "embedding_model/OllamaEmbedding.hpp"
#include "embedding_model/OpenAIEmbedding.hpp"
#include "ingestor/SingleFileIngestor.hpp"
#include "ingestor/PDFFileIngestor.hpp"
#include "ingestor/DOCXFileIngestor.hpp"
#include "retrieval/ChunkedMultiVectorRetriever.hpp"
#include "store/duckdb/DuckDBDocStore.hpp"
#include "store/duckdb/DuckDBVectorStore.hpp"
#include "tools/http/OpenAICompatibleAPIServer.hpp"
#include "tools/Assertions.hpp"


namespace insintct::exmaples::chat_doc {
    using namespace INSTINCT_RETRIEVAL_NS;
    using namespace INSTINCT_SERVER_NS;
    using namespace INSTINCT_LLM_NS;


    struct LLMProviderOptions {
        std::string provider_name = "ollama";
        std::string host;
        int port = 0;
        std::string api_key;

        OpenAIConfiguration openai;
        OllamaConfiguration ollama;
    };

    struct BuildCommandOptions {
        std::string filename;
        std::string file_type = "TXT";
        DuckDBStoreOptions doc_store;
        DuckDBStoreOptions vector_store;
        LLMProviderOptions llm_provider;
        bool force_rebuild = false;
    };

    struct ServeCommandOptions {
        LLMProviderOptions llm_provider;
        // TODO DBStore refactoring to make it possible to share duckdb instantce.
        DuckDBStoreOptions doc_store;
        DuckDBStoreOptions vector_store;
        ServerOptions server;
    };

    static EmbeddingsPtr CreateEmbeddingModel(const LLMProviderOptions& options) {
        if (options.provider_name == "ollama") {
            return CreateOllamaEmbedding(options.ollama);
        }
        if (options.provider_name == "openai") {
            return CreateOpenAIEmbeddingModel(options.openai);
        }
        return nullptr;
    }

    static ChatModelPtr CreateChatModel(const LLMProviderOptions& options) {
        if (options.provider_name == "ollama") {
            return CreateOllamaChatModel(options.ollama);
        }
        if (options.provider_name == "openai") {
            return CreateOpenAIChatModel(options.openai);
        }
        return nullptr;
    }

    static void BuildCommand(const BuildCommandOptions& options) {
        IngestorPtr ingestor;
        if (options.file_type == "PDF") {
            ingestor = CreatePDFFileIngestor(options.filename);
        } else if (options.file_type == "DOCX") {
            ingestor = CreateDOCXFileIngestor(options.filename);
        } else {
            ingestor = CreatePlainTextFileIngestor(options.filename);
        }

        // force rebuild
        if (options.force_rebuild) {
            LOG_INFO("Force rebuild is enabled.");
            if(std::filesystem::remove(options.doc_store.db_file_path)) {
                LOG_INFO("DocStore file at {} is deleted", options.doc_store.db_file_path);
            }
            if (std::filesystem::remove(options.vector_store.db_file_path)) {
                LOG_INFO("VectorStore file at {} is deleted", options.vector_store.db_file_path);
            }
        } else {
            assert_true(!std::filesystem::exists(options.doc_store.db_file_path), "DocStore file already exists. Abort to prevent data corruption.");
            assert_true(!std::filesystem::exists(options.vector_store.db_file_path), "VectorStore file already exists. Abort to prevent data corruption.");
        }

        const auto doc_store = CreateDuckDBDocStore(options.doc_store, CreatePresetMetdataSchema());

        EmbeddingsPtr embedding_model = CreateEmbeddingModel(options.llm_provider);
        assert_true(embedding_model, "should have assigned correct embedding model");

        const auto vectore_store = CreateDuckDBVectorStore(embedding_model, options.vector_store);

        const auto child_spliter = CreateRecursiveCharacterTextSplitter();

        const auto retriever = CreateChunkedMultiVectorRetriever(
            doc_store,
            vectore_store,
            child_spliter
        );
        retriever->Ingest(ingestor->Load());
        LOG_INFO("Database is built successfully!");
    }

    static void ServeCommand(const ServeCommandOptions& options) {
        EmbeddingsPtr embedding_model = CreateEmbeddingModel(options.llm_provider);
        assert_true(embedding_model, "should have assigned correct embedding model");

        const auto doc_store = CreateDuckDBDocStore(options.doc_store);
        const auto vectore_store = CreateDuckDBVectorStore(embedding_model, options.vector_store);

        const auto child_spliter = CreateRecursiveCharacterTextSplitter();
        const auto retriever = CreateChunkedMultiVectorRetriever(
            doc_store,
            vectore_store,
            child_spliter
        );


        const auto chat_model = CreateChatModel(options.llm_provider);
        assert_true(chat_model, "should have assigned correct chat model");

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
                                           {"question", xn::steps::passthrough()},
                                           {
                                               "chat_history",
                                               // expecting `chat_history` from input of `MappingData`.
                                               xn::steps::selection("chat_history") | xn::steps::combine_chat_history()
                                           }
                                       }) | question_prompt_template | chat_model->AsModelfunction() |
                                       xn::steps::stringify_generation()
            },
            {
                // expecting `question` from input of `MappingData`
                "question", xn::steps::selection("question")
            }
        });

        const auto context_fn = xn::steps::mapping({
            {"context", xn::steps::selection("question") | retriever->AsContextRetrieverFunction()},
            {"standalone_question", xn::steps::selection("standalone_question")},
            {"question", xn::steps::selection("question")},
        });

        const auto rag_chain = question_fn | context_fn | answer_prompt_template | chat_model->AsModelfunction();

        const auto server_chain = CreateOpenAIServerChain(rag_chain);

        OpenAICompatibleAPIServer server(server_chain, options.server);
        server.StartAndWait();
    }


    static void BuildDocstoreOptionGroup(CLI::Option_group* doc_store_ogroup,
                                      DuckDBStoreOptions& duck_db_options) {
        doc_store_ogroup
                ->add_option("--doc_db_path", duck_db_options.db_file_path, "File path to database file, which will be created if given file doesn't exist.");
        doc_store_ogroup
                ->add_option("--doc_table_name", duck_db_options.table_name, "Table name for documents")
                ->default_val("doc_table");
    }

    static void BuildVecstoreOptiongroup(CLI::Option_group* vec_store_ogroup,
                                      DuckDBStoreOptions& duck_db_options) {
        vec_store_ogroup
                ->add_option("--vector_db_path", duck_db_options.db_file_path, "File path to database file, which will be created if given file doesn't exist.");
        vec_store_ogroup
                ->add_option("--vector_table_dimension", duck_db_options.dimension, "Dimension of embedding vector.")
                ->required();
        vec_store_ogroup
                ->add_option("--vector_table_name", duck_db_options.table_name, "Table name for embedding table.")
                ->default_val("embedding_table");
    }

    static void BuildLLMProviderOptionGroup(CLI::Option_group* llm_provider_ogroup,
                                          LLMProviderOptions& provider_options) {
        llm_provider_ogroup->add_option("--llm_provider", provider_options.provider_name,
                                        "Specify LLM to use for both embedding and chat completion. Ollama, OpenAI API, or any OpenAI API compatible servers are supported.")
                ->check(CLI::IsMember({"ollama", "openai"}, CLI::ignore_case))
                ->required();

        const std::map<std::string, HttpProtocol> protocol_map{
            {"http", kHTTP},
            {"https", kHTTPS}
        };
        const auto openai_ogroup = llm_provider_ogroup->add_option_group("openai");
        openai_ogroup->add_option("--openai_api_key", provider_options.openai.api_key);
        openai_ogroup->add_option("--openai_host", provider_options.openai.endpoint.host)
                ->default_val(OPENAI_DEFAULT_ENDPOINT.host);
        openai_ogroup->add_option("--openai_port", provider_options.openai.endpoint.port)
                ->default_val(OPENAI_DEFAULT_ENDPOINT.port);
        openai_ogroup->add_option("--openai_protocol", provider_options.openai.endpoint.protocol)
                ->transform(CLI::CheckedTransformer(protocol_map, CLI::ignore_case))
                ->default_val(OPENAI_DEFAULT_ENDPOINT.protocol);

        const auto ollama_ogroup = llm_provider_ogroup->add_option_group("ollama");
        ollama_ogroup->add_option("--ollama_host", provider_options.ollama.endpoint.host)
                ->default_val(OLLAMA_ENDPOINT.host);
        ollama_ogroup->add_option("--ollama_port", provider_options.ollama.endpoint.port)
                ->default_val(OLLAMA_ENDPOINT.port);
        ollama_ogroup->add_option("--ollama_protocol", provider_options.ollama.endpoint.protocol)
                ->transform(CLI::CheckedTransformer(protocol_map, CLI::ignore_case))
                ->default_val(OLLAMA_ENDPOINT.protocol);
    }
}


int main(int argc, char** argv) {
    using namespace CLI;
    using namespace insintct::exmaples::chat_doc;
    App app{
        "😊 ChatDoc: Chat with your documents locally with privacy. "
    };
    argv = app.ensure_utf8(argv);
    app.set_help_all_flag("--help-all", "Expand all help");

    // requires at least one sub-command
    app.require_subcommand(1);

    // llm_provider_options for both chat model and embedding model
    LLMProviderOptions llm_provider_options;
    BuildLLMProviderOptionGroup(app.add_option_group("LLM"), llm_provider_options);

    // build command
    BuildCommandOptions build_command_options;
    const auto build_command = app.add_subcommand(
        "build", "💼 Anaylize a single document and build database of learned context data");
    build_command->add_flag("--force", build_command_options.force_rebuild, "A flag to force rebuild of database, which means existing db files will be deleted. Use this option with caution!");
    build_command->add_option("-f,--file", build_command_options.filename, "Path to the document you want analyze")
            ->required()
            ->check(ExistingFile);
    build_command
            ->add_option("-t,--type", build_command_options.file_type,
                         "File format of assigned document. Supported types are PDF,TXT,MD,DOCX")
            ->default_val("TXT")
            ->check(IsMember({"PDF", "DOCX", "MD", "TXT"}, CLI::ignore_case));
    BuildDocstoreOptionGroup(
        build_command->add_option_group("docstore"),
        build_command_options.doc_store);
    BuildVecstoreOptiongroup(
        build_command->add_option_group("vecstore"),
        build_command_options.vector_store);
    build_command->final_callback([&]() {
        build_command_options.llm_provider = llm_provider_options;
    });

    // serve command
    ServeCommandOptions serve_command_options;
    const auto serve_command = app.add_subcommand(
        "serve", "💃 Start a OpenAI API compatible server with database of learned context");
    serve_command
            ->add_option("-p,--port", serve_command_options.server.port, "Port number which API server will listen")
            ->default_val("9090");
    BuildDocstoreOptionGroup(
        serve_command->add_option_group("docstore"),
        serve_command_options.doc_store);
    BuildVecstoreOptiongroup(
        serve_command->add_option_group("vecstore"),
        serve_command_options.vector_store);

    build_command->final_callback([&]() {
        serve_command_options.llm_provider = llm_provider_options;
    });

    // log level
    bool enable_verbose_log;
    app.add_flag("-v,--verbose", enable_verbose_log, "A flag to enable verbose log");

    // parse input
    CLI11_PARSE(app, argc, argv);

    // setup logging
    if(enable_verbose_log) {
        fmtlog::setLogLevel(fmtlog::DBG);
        LOG_INFO("Verbose logging is enabled.");
    } else {
        LOG_INFO("Logging level is default to INFO");
    }
    fmtlog::startPollingThread();

    // trigger sub-command
    for(const auto* sub_command: app.get_subcommands()) {
        try {
            if (sub_command->get_name() == "build") {
                BuildCommand(build_command_options);
            }
            if(sub_command->get_name() == "serve") {
                ServeCommand(serve_command_options);
            }
        } catch (const std::runtime_error& err) {
            LOG_ERROR("Failed to execute sub-command {} due to exception. Possible reason: {}", sub_command->get_name(), err.what());
        }
    }

    return 0;
}
