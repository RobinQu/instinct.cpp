//
// Created by RobinQu on 2024/3/26.
//

#include <CLI/CLI.hpp>
#include <document/RecursiveCharacterTextSplitter.hpp>

#include "chain/RAGChain.hpp"
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

    struct ServeCommandOptions {
        std::string filename;
        std::string type;
        int port_number = 0;
    };

    static void ServeCommand(const ServeCommandOptions& options) {
        IngestorPtr ingestor;
        if(options.type == "PDF") {
            ingestor = CreatePDFFileIngestor(options.filename);
        } else if(options.type == "DOCX") {
            ingestor = CreateDOCXFileIngestor(options.filename);
        } else {
            ingestor = CreatePlainTextFileIngestor(options.filename);
        }

        auto doc_store = CreateDuckDBDocStore({.table_name = "doc_store", .in_memory = true});
        EmbeddingsPtr embedding_model;
        auto vectore_store = CreateDuckDBVectorStore(embedding_model, {.table_name = "doc_embedding_store", .in_memory = true});

        auto child_spliter = CreateRecursiveCharacterTextSplitter();

        auto retriever = CreateChunkedMultiVectorRetriever(
            doc_store,
            vectore_store,
            child_spliter
            );

        retriever->Ingest(ingestor->Load());

        OpenAIConfiguration configuration;
        auto chat_model = CreateOpenAIChatModel(configuration);


        auto question_prompt_template = CreatePlainPromptTemplate(R"(
Given the following conversation and a follow up question, rephrase the follow up question to be a standalone question, in its original language.
Chat History:
{chat_history}
Follow Up Input: {question}
Standalone question:)",
             {
                 .input_keys = {"chat_history", "question"},
             });
        auto answer_prompt_template = CreatePlainPromptTemplate(
                R"(Answer the question based only on the following context:
{context}

Question: {standalone_question}

)", {.input_keys = {"context", "standalone_question"}});

        auto question_fn = xn::steps::mapping({
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

        auto context_fn = xn::steps::mapping({
            {"context", xn::steps::selection("question") | retriever->AsContextRetrieverFunction()},
            {"standalone_question", xn::steps::selection("standalone_question")},
            {"question", xn::steps::selection("question")},
        });

        auto rag_chain = question_fn | context_fn | answer_prompt_template | chat_model->AsModelfunction();

        auto server_chain = CreateOpenAIServerChain(rag_chain);

        OpenAICompatibleAPIServer server(server_chain, {.port = options.port_number});
        server.StartAndWait();
    }

}


int main(int argc, char** argv) {
    using namespace CLI;
    using namespace insintct::exmaples::chat_doc;
    App app {"ChatDoc: Chat with your documents. It's a privacy-first application that won't interact with cloud services."};
    argv = app.ensure_utf8(argv);

    auto serve_command = app.add_subcommand("serve", "Anaylize a single document and start a OpenAI API compatible server")
        ->require_subcommand(); // requires at least one sub-command

    ServeCommandOptions command;
    serve_command->add_option("-f,--file", command.filename, "Path to the document you want analyze")
        ->required()
        ->check(ExistingFile);
    serve_command
        ->add_option("-t,--type", command.type, "File format of assigned document. Supported types are PDF,TXT,MD,DOCX")
        ->default_val("PDF")
        ->check(IsMember({"PDF", "DOCX", "MD", "TXT"}));
    serve_command->add_option("-p,--port", command.port_number, "Port number which API server will listen");
    auto embedding_options = serve_command->add_option_group("embedding_api", "Options for embedding API");
    embedding_options->add_option("-o,--provider", "Provider type");




}