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

        TempFile(TempFile&& other) noexcept {
            path = std::move(other.path);
            file = std::move(other.file);
        }

        TempFile(): path(std::filesystem::current_path() / StringUtils::GenerateUUIDString()), file(path, std::ios::in | std::ios::out | std::ios::trunc) {}

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
