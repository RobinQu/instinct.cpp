//
// Created by RobinQu on 2024/4/25.
//

#ifndef FILESYSTEMOBJECTSTORE_HPP
#define FILESYSTEMOBJECTSTORE_HPP

#include <fstream>

#include "DataGlobals.hpp"
#include "IObjectStore.hpp"
#include "tools/IOUtils.hpp"

namespace INSTINCT_DATA_NS {
    using FileSystemObjectKeyMapper = std::function<std::filesystem::path(const std::filesystem::path& root, const std::string &bucket_name, const std::string &object_key)>;


    static FileSystemObjectKeyMapper DEFAULT_OBJECT_KEY_MAPPER = [](const std::filesystem::path& root, const std::string &bucket_name, const std::string &object_key) {
        return root / bucket_name / object_key;
    };

    class FileSystemObjectStore final: public IObjectStore<> {
        std::filesystem::path root_directory_;
        FileSystemObjectKeyMapper key_mapper_;

    public:
        explicit FileSystemObjectStore(std::filesystem::path root_directory, FileSystemObjectKeyMapper key_mapper = DEFAULT_OBJECT_KEY_MAPPER)
            : root_directory_(std::move(root_directory)),
              key_mapper_(std::move(key_mapper)) {
            std::filesystem::create_directories(root_directory_);
            assert_true(std::filesystem::exists(root_directory_), "directory for FileSystemObjectStore should be created correctly.");
            LOG_INFO("Init FileSystemObjectStore at {}", root_directory_);
        }

        OSSStatus PutObject(const std::string &bucket_name, const std::string &object_key,
                            std::istream &input_stream) override {
            LOG_DEBUG("PutObject with stream: bucket_name={}, object_key={}", bucket_name, object_key);
            const auto object_path = EnsureObjectPath_(bucket_name, object_key);
            std::ofstream object_file(object_path, std::ios::binary | std::ios::out | std::ios::trunc);
            OSSStatus status;
            if (!object_file.is_open()) {
                status.set_error_type(OSSStatus_ErrorType_FileNotOpen);
                status.set_has_error(true);
            } else {
                object_file << input_stream.rdbuf();
                LOG_DEBUG("PutObject with stream: final pos {}, dest {}", std::to_string(input_stream.tellg()), object_path);
            }
            object_file.close();

            if (std::filesystem::file_size(object_path) == 0) {
                std::filesystem::remove(object_path);
                status.set_error_type(OSSStatus_ErrorType_EmptyFile);
                status.set_has_error(true);
            }
            return status;
        }

        OSSStatus PutObject(const std::string &bucket_name, const std::string &object_key,
            const std::string &buffer) override {
            LOG_DEBUG("PutObject with buffer: bucket_name={}, object_key={}", bucket_name, object_key);
            const auto object_path = EnsureObjectPath_(bucket_name, object_key);
            std::ofstream object_file(object_path, std::ios::binary | std::ios::out | std::ios::trunc);
            object_file << buffer;
            return {};
        }

        OSSStatus GetObject(const std::string &bucket_name, const std::string &object_key,
            std::ostream &output_stream) override {
            LOG_DEBUG("GetObject with stream: bucket_name={}, object_key={}", bucket_name, object_key);
            const auto object_path = EnsureObjectPath_(bucket_name, object_key);
            OSSStatus status;
            if (!std::filesystem::exists(object_path)) {
                status.set_has_error(true);
                status.set_error_type(OSSStatus_ErrorType_ObjectNotFound);
                return status;
            }
            std::ifstream object_file(object_path, std::ios::binary | std::ios::in);
            if (object_file.is_open()) {
                output_stream << object_file.rdbuf();
                LOG_DEBUG("GetObject with stream, final position: {} source: {}", std::to_string(output_stream.tellp()), object_path);
                object_file.close();
                return status;
            }
            status.set_has_error(true);
            status.set_error_type(OSSStatus_ErrorType_FileNotOpen);
            return status;
        }

        OSSStatus GetObject(const std::string &bucket_name, const std::string &object_key,
            std::string &buffer) override {
            LOG_DEBUG("GetObject with buffer: bucket_name={}, object_key={}", bucket_name, object_key);
            const auto object_path = EnsureObjectPath_(bucket_name, object_key);
            OSSStatus status;
            if (!std::filesystem::exists(object_path)) {
                status.set_has_error(true);
                status.set_error_type(OSSStatus_ErrorType_ObjectNotFound);
                return status;
            }
            buffer = IOUtils::ReadString(object_path);
            return status;
        }

        OSSStatus DeleteObject(const std::string &bucket_name, const std::string &object_key) override {
            LOG_DEBUG("DeleteObject: bucket_name={}, object_key={}", bucket_name, object_key);
            const auto object_path = EnsureObjectPath_(bucket_name, object_key);
            OSSStatus status;
            if (!std::filesystem::exists(object_path)) {
                status.set_has_error(true);
                status.set_error_type(OSSStatus_ErrorType_ObjectNotFound);
                return status;
            }
            std::filesystem::remove(object_path);
            return status;
        }

        ObjectState GetObjectState(const std::string &bucket_name, const std::string &object_key) override {
            const auto object_path = EnsureObjectPath_(bucket_name, object_key);
            ObjectState state;
            auto* status = state.mutable_status();
            if (!std::filesystem::exists(object_path)) {
                status->set_has_error(true);
                status->set_error_type(OSSStatus_ErrorType_ObjectNotFound);
                return state;
            }
            state.set_byte_size(static_cast<int32_t>(std::filesystem::file_size(object_path)));
            return state;
        }

    private:
        [[nodiscard]] std::filesystem::path EnsureObjectPath_(const std::string &bucket_name, const std::string &object_key) const {
            const auto object_path = key_mapper_(root_directory_, bucket_name, object_key);
            std::filesystem::create_directories(object_path.parent_path());
            return object_path;
        }
    };
}

#endif //FILESYSTEMOBJECTSTORE_HPP
