//
// Created by RobinQu on 2024/6/3.
//

#ifndef RETRIEVEROBJECTFACTORY_HPP
#define RETRIEVEROBJECTFACTORY_HPP
#include <regex>
#include "RetrievalGlobals.hpp"
#include "ingestor/BaseIngestor.hpp"
#include "ingestor/DirectoryTreeIngestor.hpp"
#include "ingestor/DOCXFileIngestor.hpp"
#include "ingestor/PDFFileIngestor.hpp"
#include "ingestor/SingleFileIngestor.hpp"

namespace INSTINCT_RETRIEVAL_NS {

    class RetrieverObjectFactory final {
    public:
        static IngestorPtr CreateIngestor(const std::filesystem::path& file_path, bool fail_fast = false) {
            assert_true(std::filesystem::exists(file_path), "file should exist");
            assert_true(is_regular_file(file_path), "path should be pointed to a regular file");
            static std::regex EXT_NAME_PATTERN { R"(.+\.(.+))"};
            // we rely on extname to choose ingestor
            const std::string path_string = file_path.string();
            if (std::smatch match; std::regex_match(path_string, match, EXT_NAME_PATTERN)) {
                if (match.size() == 2) {
                    const auto extname = StringUtils::ToLower(match[1].str());
                    if (extname == "pdf") {
                        return CreatePDFFileIngestor(file_path);
                    }
                    if (extname == "txt" || extname == "md") {
                        return CreatePlainTextFileIngestor(file_path);
                    }
                    if (extname == "docx") {
                        return CreateDOCXFileIngestor(file_path);
                    }
                }
            }
            if (fail_fast) {
                throw InstinctException(fmt::format("Cannot build ingestor for file {}", path_string));
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
                ingestor_factory_function = [](const std::filesystem::path& path) {return  CreateIngestor(path);};
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
