//
// Created by RobinQu on 2024/3/26.
//

#include <CLI/CLI.hpp>
#include <document/RecursiveCharacterTextSplitter.hpp>

#include "LLMTestGlobals.hpp"
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
            return  CreateOllamaEmbedding(options.ollama);
        }
        if (options.provider_name == "openai") {
            return  CreateOpenAIEmbeddingModel(options.openai);
        }
        // if (options.llm_provider.provider_name == "pesudo") {
        //     embedding_model =  instinct::test::create_pesudo_embedding_model();
        // }
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
        if(options.file_type == "PDF") {
            ingestor = CreatePDFFileIngestor(options.filename);
        } else if(options.file_type == "DOCX") {
            ingestor = CreateDOCXFileIngestor(options.filename);
        } else {
            ingestor = CreatePlainTextFileIngestor(options.filename);
        }

        const auto doc_store = CreateDuckDBDocStore(options.doc_store);

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

}


static void build_docstore_ogroup(CLI::Option_group* doc_store_ogroup, instinct::retrieval::DuckDBStoreOptions& duck_db_options) {
    doc_store_ogroup->add_option("-p,--db_path",  duck_db_options.db_file_path, "File path to database file");
    doc_store_ogroup->add_option("-t,--table_name",  duck_db_options.table_name, "Table name for documents")->default_str("doc_table");

    // doc_store_ogroup->add_flag("-m,--memory", duck_db_options.in_memory, "Flag to indicate use build in-memory DB instantce");
}

static void build_vecstore_ogroup(CLI::Option_group* vec_store_ogroup, instinct::retrieval::DuckDBStoreOptions& duck_db_options) {
    vec_store_ogroup->add_option("-p,--db_path",  duck_db_options.db_file_path, "File path to database file")->check(CLI::ExistingFile);
    vec_store_ogroup->add_option("-d,--dimension", duck_db_options.dimension, "Dimension of embedding vector");
    vec_store_ogroup->add_option("-t,--table_name",  duck_db_options.table_name, "Table name for embedding table")->default_str("embedding_table");
}

static void build_llm_provider_ogroup(CLI::Option_group* llm_provider_ogroup, insintct::exmaples::chat_doc::LLMProviderOptions& provider_options) {
    llm_provider_ogroup->add_option("-p,--provider_name", provider_options.provider_name, "Specify LLM to use for both embedding and chat completion. Ollama, OpenAI API, or any OpenAI API compatible servers are supported.")
    ->check(CLI::IsMember({"ollama", "openai"}, CLI::ignore_case));
    llm_provider_ogroup->add_option("--api_key", provider_options.api_key, "API key for service provider.");
    llm_provider_ogroup->add_option("--host", provider_options.host, "Host for API service, without trailing slash, e.g. https://api.openai.com");
    llm_provider_ogroup->add_option("--port", provider_options.port)->default_val(80);
    llm_provider_ogroup->parse_complete_callback([&]() {
        // merge similar options and copy those options respectively
        if (provider_options.provider_name == "ollama") {
            provider_options.ollama.endpoint.host = provider_options.host;
            provider_options.ollama.endpoint.port = provider_options.port;
        }
        if (provider_options.provider_name == "openai") {
            provider_options.openai.endpoint.host = provider_options.host;
            provider_options.openai.endpoint.port = provider_options.port;
            provider_options.openai.api_key = provider_options.api_key;
        }
    });
}


int main(int argc, char** argv) {
    using namespace CLI;
    using namespace insintct::exmaples::chat_doc;
    App app {"ChatDoc: Chat with your documents. It's a privacy-first application that won't interact with cloud services."};
    argv = app.ensure_utf8(argv);

    // requires at least one sub-command
    app.require_subcommand();

    // llm_provider_options for both chat model and embedding model
    LLMProviderOptions llm_provider_options;
    build_llm_provider_ogroup(app.add_option_group("llm_provider"), llm_provider_options);

    // build command
    BuildCommandOptions build_command_options;
    const auto build_command = app.add_subcommand("build", "Anaylize a single document and build database of learned context data");
    build_command->add_option("-f,--file", build_command_options.filename, "Path to the document you want analyze")
        ->required()
        ->check(ExistingFile);
    build_command
        ->add_option("-t,--type", build_command_options.file_type, "File format of assigned document. Supported types are PDF,TXT,MD,DOCX")
        ->default_val("TXT")
        ->check(IsMember({"PDF", "DOCX", "MD", "TXT"}, CLI::ignore_case));
    build_docstore_ogroup(
        build_command->add_option_group("docstore"),
        build_command_options.doc_store);
    build_vecstore_ogroup(
        build_command->add_option_group("vecstore"),
        build_command_options.vector_store);
    build_command->final_callback([&]() {
        build_command_options.llm_provider = llm_provider_options;
        BuildCommand(build_command_options);
    });

    // serve command
    ServeCommandOptions serve_command_options;
    const auto serve_command = app.add_subcommand("serve", "Start a OpenAI API compatible server with database of learned context");
    serve_command
        ->add_option("-p,--port", serve_command_options.server.port, "Port number which API server will listen")
        ->default_val("9090");
    build_docstore_ogroup(
        serve_command->add_option_group("docstore"),
        build_command_options.doc_store);
    build_vecstore_ogroup(
        serve_command->add_option_group("vecstore"),
        build_command_options.vector_store);

    build_command->final_callback([&]() {
        serve_command_options.llm_provider = llm_provider_options;
        ServeCommand(serve_command_options);
    });

    CLI11_PARSE(app, argc, argv);
    return 0;
}