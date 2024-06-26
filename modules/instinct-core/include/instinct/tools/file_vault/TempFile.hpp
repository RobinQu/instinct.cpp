//
// Created by RobinQu on 2024/4/29.
//

#ifndef TEMPFILE_HPP
#define TEMPFILE_HPP
#include <fstream>

#include <instinct/CoreGlobals.hpp>
#include <instinct/tools/StringUtils.hpp>

namespace INSTINCT_CORE_NS {
    /**
     * Disposable temporary file object
     */
    class TempFile {
    public:
        std::filesystem::path path;
        std::fstream file;

        TempFile(const TempFile&)=delete;

        explicit TempFile(const std::string& filename = StringUtils::GenerateUUIDString()):
            path(std::filesystem::temp_directory_path() / StringUtils::GenerateUUIDString() / filename) {
            if(!std::filesystem::exists(path.parent_path())) {
                std::filesystem::create_directories(path.parent_path());
            }
            file = std::fstream(path, std::ios::in | std::ios::out | std::ios::trunc);
            assert_true(file.is_open(), "file should be opened");
            LOG_DEBUG("Create temp file at {}", path);
        }

        TempFile(TempFile&& other) noexcept {
            path = std::move(other.path);
            file = std::move(other.file);
        }


        ~TempFile() {
            LOG_DEBUG("Destroy temp file at {}", path);
            if (file) {
                file.close();
            }
            if (std::filesystem::exists(path)) {
                std::filesystem::remove(path);
            }
        }
    };
}

#endif //TEMPFILE_HPP
