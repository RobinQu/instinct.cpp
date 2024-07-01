//
// Created by RobinQu on 2024/3/13.
//

#ifndef TEXTFILEINGESTOR_HPP
#define TEXTFILEINGESTOR_HPP


#include <instinct/ingestor/ingestor.hpp>
#include <filesystem>
#include <fstream>

#include <instinct/retrieval/chunked_multi_vector_retriever.hpp>


namespace INSTINCT_RETRIEVAL_NS {
    /**
     * Datasource for single file. Current implementation will parse file at `file_path` as plain text file. Subclasses are welcomed to implement custom parsing.
     */
    class SingleFileIngestor final: public BaseIngestor {
        std::filesystem::path file_path_;
        std::string file_source_;

    public:
        explicit SingleFileIngestor(std::filesystem::path file_path, const DocumentPostProcessor &document_post_processor = nullptr, std::string file_source = "")
            : BaseIngestor(document_post_processor), file_path_(std::move(file_path)), file_source_(std::move(file_source)) {
        }

        AsyncIterator<Document> Load() override {
            // https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
            // https://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
            // following code seems to be most efficient way to read large text file. it's ironically funny there are so many ways to read a text file even in modern C++ standards, and we have to consult benchmarks to find a reliable approach.
            std::ifstream t(file_path_);
            std::stringstream buffer;
            buffer << t.rdbuf();
            return rpp::source::just(CreateNewDocument(
                buffer.str(),
                ROOT_DOC_ID,
                1,
                StringUtils::IsBlankString(file_source_) ? file_path_.string() : file_source_
            ));
        }
    };

    static IngestorPtr CreatePlainTextFileIngestor(const std::filesystem::path& file_path, const DocumentPostProcessor &document_post_processor = nullptr, const std::string& file_source = "") {
        return std::make_shared<SingleFileIngestor>(file_path, document_post_processor, file_source);
    }
}

#endif //TEXTFILEINGESTOR_HPP
