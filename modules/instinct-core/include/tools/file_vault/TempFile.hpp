//
// Created by RobinQu on 2024/4/29.
//

#ifndef TEMPFILE_HPP
#define TEMPFILE_HPP
#include <fstream>

#include "CoreGlobals.hpp"
#include "tools/StringUtils.hpp"

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
            path(std::filesystem::temp_directory_path() / StringUtils::GenerateUUIDString() / filename),
            file(path, std::ios::in | std::ios::out | std::ios::trunc) {
            if(!std::filesystem::exists(path.parent_path())) {
                std::filesystem::create_directories(path.parent_path());
            }
        }

        TempFile(TempFile&& other) noexcept {
            path = std::move(other.path);
            file = std::move(other.file);
        }


        ~TempFile() {
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
