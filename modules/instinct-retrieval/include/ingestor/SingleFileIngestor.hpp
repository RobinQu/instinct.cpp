//
// Created by RobinQu on 2024/3/13.
//

#ifndef TEXTFILEINGESTOR_HPP
#define TEXTFILEINGESTOR_HPP


#include "BaseIngestor.hpp"
#include <filesystem>
#include <fstream>

#include "retrieval/ChunkedMultiVectorRetriever.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    /**
     * Datasource for single file. Current implementation will parse file at `file_path` as plain text file. Subclasses are welcomed to impmenent custom parsing.
     */
    class SingleFileIngestor final: public BaseIngestor {
        std::filesystem::path file_path_;
    public:
        explicit SingleFileIngestor(std::filesystem::path file_path)
            : file_path_(std::move(file_path)) {
        }

        AsyncIterator<Document> Load() override {
            // https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
            // https://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
            // following code seems to be most efficient way to read large text file. it's ironically funny there are so many ways to read a text file even in modern C++ standards, and we have to consult benchmarks to find a reliable approach.
            std::ifstream t(file_path_);
            std::stringstream buffer;
            buffer << t.rdbuf();
            Document document;
            DocumentUtils::AddPresetMetadataFileds(document, ROOT_DOC_ID);
            document.mutable_text()->assign(buffer.str());
            return rpp::source::just(document);

        }
    };

    static IngestorPtr CreatePlainTextFileIngestor(const std::filesystem::path& file_path) {
        return std::make_shared<SingleFileIngestor>(file_path);
    }
}

#endif //TEXTFILEINGESTOR_HPP
