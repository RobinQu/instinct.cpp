//
// Created by RobinQu on 2024/3/13.
//

#ifndef DIRECTORYTREEINGESTOR_HPP
#define DIRECTORYTREEINGESTOR_HPP


#include "BaseIngestor.hpp"
#include "tools/Assertions.hpp"
#include <filesystem>

#include "SingleFileIngestor.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace U_ICU_NAMESPACE;

    using IngestorFactoryFunction = std::function<IngestorPtr(const std::filesystem::path& entry_path)>;

    static IngestorFactoryFunction DEFAULT_INGESTOR_FACTORY_FUNCTION = [](const std::filesystem::path& entry_path) {
        return std::make_shared<SingleFileIngestor>(entry_path);
    };



    /**
     * DirectoryTreeIngestor is capable of turning regular files in a directory into split documents.
     */
    class DirectoryTreeIngestor: public BaseIngestor {
        std::filesystem::path folder_path_;
        std::unique_ptr<RegexMatcher> regex_matcher_{};
        IngestorFactoryFunction ingestor_factory_function_ = DEFAULT_INGESTOR_FACTORY_FUNCTION;
        bool recursive_;
    public:

        explicit DirectoryTreeIngestor(std::filesystem::path folder_path, const bool recursive = true)
            : folder_path_(std::move(folder_path)), recursive_(recursive) {
            assert_true(std::filesystem::exists(folder_path_), "Given folder should exist");
        }

        /**
         * 
         * @param regex_string regex string to bulid a matcher for filename
         * @param folder_path
         * @param recursive a flag to indicate whether it's intended to iterate given `folder_path` recursively or not
         */
        DirectoryTreeIngestor(
            const UnicodeString& regex_string,
            const std::filesystem::path& folder_path,
            const bool recursive = true
            ):  DirectoryTreeIngestor(folder_path, recursive) {
            UErrorCode status = U_ZERO_ERROR;
            regex_matcher_ = std::make_unique<RegexMatcher>(regex_string, 0, status);
            assert_icu_status(status, "Failed to build RegexMatcher using string: " + regex_string);
        }

        AsyncIterator<Document> Load() override {
            std::vector<std::filesystem::path> valid_files;
            if (recursive_) {
                for (const auto& dir_entry: std::filesystem::recursive_directory_iterator{folder_path_}) {
                    if (MatchSingleEntry_(dir_entry)) {
                        valid_files.push_back(dir_entry.path());
                    }
                }
            } else {
                for (auto const& dir_entry : std::filesystem::directory_iterator{folder_path_}) {
                    if (MatchSingleEntry_(dir_entry)) {
                        valid_files.push_back(dir_entry.path());
                    }
                }
            }

            // TODO Do it in parallel
            return rpp::source::from_iterable(valid_files)
                | rpp::operators::flat_map([&](const std::filesystem::path& entry) {
                    auto ingestor = ingestor_factory_function_(entry);
                    return ingestor->Load();
                });
        }

    private:
        [[nodiscard]] bool MatchSingleEntry_(const std::filesystem::directory_entry& dir_entry) const {
            // only accepting regular file, skipping directories, symlinks and block files, etc.
            if (dir_entry.is_regular_file()) {
                if (this->regex_matcher_) {
                    const UnicodeString absolute_path = UnicodeString::fromUTF8(dir_entry.path().string());
                    this->regex_matcher_->reset(absolute_path);
                    return this->regex_matcher_->find();
                }
                return true;
            }
            return false;
        }
    };


    static IngestorPtr CreateDirectoryTreeIngestor(const std::filesystem::path& folder, bool recursive = true) {
        return std::make_shared<DirectoryTreeIngestor>(folder, recursive);

    }
}

#endif //DIRECTORYTREEINGESTOR_HPP
