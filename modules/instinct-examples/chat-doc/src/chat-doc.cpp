//
// Created by RobinQu on 2024/3/26.
//

#include <CLI/CLI.hpp>
#include "ingestor/SingleFileIngestor.hpp"
namespace insintct::exmaples::chat_doc {


    struct ServeCommandOptions {
        std::string filename;
        std::string type;
        int port_number = 0;
    };

    static void ServeCommand(const ServeCommandOptions& options) {

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
        ->check(IsMember{"PDF", "DOCX", "MD", "TXT"});
    serve_command->add_option("-p,--port", command.port_number, "Port number which API server will listen");



}