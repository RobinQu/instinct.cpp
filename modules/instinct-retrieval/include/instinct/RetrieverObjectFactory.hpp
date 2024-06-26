//
// Created by RobinQu on 2024/6/3.
//

#ifndef RETRIEVEROBJECTFACTORY_HPP
#define RETRIEVEROBJECTFACTORY_HPP
#include <regex>
#include <instinct/RetrievalGlobals.hpp>
#include <instinct/ingestor/BaseIngestor.hpp>
#include <instinct/ingestor/DirectoryTreeIngestor.hpp>
#include <instinct/ingestor/DOCXFileIngestor.hpp>
#include <instinct/ingestor/ParquetFileIngestor.hpp>
#include <instinct/ingestor/PDFFileIngestor.hpp>
#include <instinct/ingestor/SingleFileIngestor.hpp>

namespace INSTINCT_RETRIEVAL_NS {

    struct IngestorOptions {
        std::filesystem::path file_path;
        std::string file_source_id;
        std::string parquet_mapping;
        DocumentPostProcessor document_post_processor;
        bool fail_fast;
    };

    class RetrieverObjectFactory final {
    public:
        static IngestorPtr CreateIngestor(const IngestorOptions& ingestor_options) {
            static std::regex EXT_NAME_PATTERN { R"(.+\.(.+))"};
            // we rely on extname to choose ingestor
            const std::string path_string = ingestor_options.file_path.string();
            if (std::smatch match; std::regex_match(path_string, match, EXT_NAME_PATTERN)) {
                if (match.size() == 2) {
                    const auto extname = StringUtils::ToLower(match[1].str());
                    return CreateIngestor(extname, ingestor_options);
                }
            }
            if (ingestor_options.fail_fast) {
                throw InstinctException(fmt::format("Cannot get type hint from path {}", path_string));
            }
            return nullptr;
        }

        static IngestorPtr CreateIngestor(
            std::string type_hint,
            const IngestorOptions& ingestor_options) {
            const auto& file_path = ingestor_options.file_path;
            const DocumentPostProcessor& document_post_processor = ingestor_options.document_post_processor;
            const auto& file_source_id = ingestor_options.file_source_id;
            assert_true(std::filesystem::exists(file_path), "file should exist");
            assert_true(is_regular_file(file_path), "path should be pointed to a regular file");

            type_hint = StringUtils::ToLower(type_hint);
            if (type_hint == "pdf") {
                return CreatePDFFileIngestor(file_path, document_post_processor, file_source_id);
            }
            if (type_hint == "txt" || type_hint == "md") {
                return CreatePlainTextFileIngestor(file_path, document_post_processor, file_source_id);
            }
            if (type_hint == "docx") {
                return CreateDOCXFileIngestor(file_path, document_post_processor, file_source_id);
            }
            if (type_hint == "parquet") {
                return CreateParquetIngestor(file_path, ingestor_options.parquet_mapping, {}, document_post_processor, file_source_id);
            }
            if (ingestor_options.fail_fast) {
                throw InstinctException(fmt::format("Cannot build ingestor for file {}", file_path.string()));
            }
            return nullptr;
        }

        static IngestorPtr CreateDirectoryTreeIngestor(
            const std::filesystem::path& folder,
            std::unique_ptr<RegexMatcher> regex_matcher = nullptr,
            IngestorFactoryFunction ingestor_factory_function = nullptr,
            bool recursive = true
        ) {
            if (!ingestor_factory_function) {
                ingestor_factory_function = [](const std::filesystem::path& path) {return CreateIngestor({
                    .file_path = path
                });};
            }
            return std::make_shared<DirectoryTreeIngestor>(
                folder,
                std::move(regex_matcher),
                ingestor_factory_function,
                recursive
            );
        }

        static IngestorPtr CreateDirectoryTreeIngestor(
            const std::filesystem::path& folder,
            const std::string& regex_string,
            const IngestorFactoryFunction& ingestor_factory_function = nullptr,
            bool recursive = true
        ) {
            assert_not_blank(regex_string);
            std::unique_ptr<RegexMatcher> matcher = nullptr;
            UErrorCode status = U_ZERO_ERROR;
            matcher = std::make_unique<RegexMatcher>(regex_string.c_str(), 0, status);
            assert_icu_status(status, fmt::format("failed to create RegexMatcher using string {}", regex_string));
            return CreateDirectoryTreeIngestor(
                folder,
                std::move(matcher),
                ingestor_factory_function,
                recursive);
        }

    };


}


#endif //RETRIEVEROBJECTFACTORY_HPP
