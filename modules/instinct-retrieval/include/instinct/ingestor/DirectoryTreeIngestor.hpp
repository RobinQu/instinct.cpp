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

    /**
     * DirectoryTreeIngestor is capable of turning regular files in a directory into split documents.
     */
    class DirectoryTreeIngestor final: public BaseIngestor {
        std::filesystem::path folder_path_;
        std::unique_ptr<RegexMatcher> regex_matcher_;
        IngestorFactoryFunction ingestor_factory_function_;
        bool recursive_;
    public:
        DirectoryTreeIngestor(
            std::filesystem::path folder_path,
            std::unique_ptr<RegexMatcher> regex_matcher,
            IngestorFactoryFunction ingestor_factory_function,
            const bool recursive
            )
            : BaseIngestor(nullptr),
            folder_path_(std::move(folder_path)), regex_matcher_(std::move(regex_matcher)), ingestor_factory_function_(std::move(ingestor_factory_function)),  recursive_(recursive) {
            assert_true(std::filesystem::exists(folder_path_), "Given folder should exist");
            assert_true(std::filesystem::is_directory(folder_path_), "should be a folder");
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
                    const auto ingestor = ingestor_factory_function_(entry);
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

}

#endif //DIRECTORYTREEINGESTOR_HPP
