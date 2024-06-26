//
// Created by RobinQu on 2024/4/18.
//

#ifndef FILESYSTEMFILEVAULT_HPP
#define FILESYSTEMFILEVAULT_HPP
#include <fstream>
#include <utility>


#include <instinct/tools/file_vault/IFileVault.hpp>
#include <instinct/tools/CodecUtils.hpp>
#include <instinct/tools/HashUtils.hpp>



namespace INSTINCT_CORE_NS {

    namespace details {
        static bool checksum(const std::filesystem::path &file_path, const ChecksumRequest &checksum_request) {
            if (checksum_request.algorithm == kNoChecksum) {
                return true;
            }

            std::string hash_value;
            if(std::ifstream data_file(file_path, std::ios::in | std::ios::binary); data_file) {
                if (checksum_request.algorithm == kMD5) {
                    hash_value = HashUtils::HashForStream<MD5>(data_file);
                }
                if (checksum_request.algorithm == kSHA1) {
                    hash_value = HashUtils::HashForStream<SHA1>(data_file);
                }
                if (checksum_request.algorithm == kSHA256) {
                    hash_value = HashUtils::HashForStream<SHA256>(data_file);
                }
            } else {
                LOG_ERROR("failed to open file at {}", file_path);
            }

            LOG_DEBUG("checksum: file={}, current={}:{}, execpted={}:{}",
                      file_path,
                      to_string(checksum_request.algorithm),
                      hash_value,
                      to_string(checksum_request.algorithm),
                      checksum_request.expected_value
                      );
            return hash_value == checksum_request.expected_value;
        }

        static void write_data_json(const FileVaultResourceEntry &entry, const std::filesystem::path &json_file_path) {
            // NOLINT(*-convert-member-functions-to-static)
            nlohmann::json data;
            data["name"] = entry.name;
            data["checksum_algorithm"] = to_string(entry.checksum_algorithm);
            data["checksum_value"] = entry.checksum_value;
            for (const auto &[k,v]: entry.metadata) {
                data["metadata"][k] = v;
            }
            std::ofstream json_file(json_file_path, std::ios::out | std::ios::trunc);
            json_file << data;
        }

        static FileVaultResourceEntry build_entry_from_json(const std::filesystem::path &data_file_path,
                                                   const std::filesystem::path &json_file_path) {
            // NOLINT(*-convert-member-functions-to-static)
            std::ifstream json_file(json_file_path);
            auto data = nlohmann::json::parse(json_file);
            FileVaultResourceEntryMetadata metadata;
            for (const auto &[k,v]: data.items()) {
                if (v.is_string()) {
                    metadata[k] = v;
                }
            }
            return {
                .name = data["name"],
                .metadata = metadata,
                .local_path = data_file_path,
                .checksum_algorithm = from_string(data["checksum_algorithm"]),
                .checksum_value = data["checksum_value"]
            };
        }


        static FileVaultResourceEntry fetch_resource(const FileVaultResourceProviderPtr &resource_provider,
                                              const std::filesystem::path &data_path,
                                              const std::filesystem::path &json_path) {
            const auto temp_file_path = std::filesystem::temp_directory_path() / StringUtils::GenerateUUIDString();

            const auto name = resource_provider->GetResourceName();

            // fetch content
            std::ofstream temp_file(temp_file_path, std::ios::out | std::ios::trunc);
            resource_provider->Persist(temp_file).get();
            temp_file.close();
            LOG_DEBUG("resource {} saved to tempfile at {}", name, temp_file_path);

            // checksum
            if (!checksum(temp_file_path, resource_provider->GetChecksum())) {
                LOG_WARN("checksum failed to resource. name={}, file_path={}. Try again immediate.", name, temp_file_path);

                // delete trash files
                std::filesystem::remove(data_path);
                std::filesystem::remove(json_path);

                // fetch again
                std::ofstream temp_file2(temp_file_path, std::ios::out | std::ios::trunc);
                resource_provider->Persist(temp_file2).wait();
                temp_file2.close();

                // if failed for second time, we will throw
                if(!checksum(temp_file_path, resource_provider->GetChecksum())) {
                    // keep file at `data_path` for debug
                    throw InstinctException(fmt::format("Checksum failed for resource content. name={},file={}", name, temp_file_path));
                }
            }

            // move to vault folder
            LOG_DEBUG("move temp file to vault. from={}, to={}", temp_file_path.string(), data_path.string());
            std::filesystem::rename(temp_file_path, data_path);

            FileVaultResourceEntry entry;
            entry.checksum_algorithm = resource_provider->GetChecksum().algorithm;
            entry.checksum_value = resource_provider->GetChecksum().expected_value;
            entry.local_path = data_path;
            entry.metadata = resource_provider->GetMetadata();
            entry.name = resource_provider->GetResourceName();

            // write metadata json
            LOG_DEBUG("write metadata json. path={}", json_path);
            write_data_json(entry, json_path);

            return entry;
        }
    }

    class FileSystemFileVault final : public IFileVault {
        std::filesystem::path root_directory_;
        std::unordered_map<std::string, FileVaultResourceProviderPtr> resources_;
        std::mutex write_mutex_;
        std::unordered_map<std::string, std::mutex> resource_mutexes_;

    public:
        explicit FileSystemFileVault(std::filesystem::path root_directory)
            : root_directory_(std::move(root_directory)) {
            assert_true(std::filesystem::exists(root_directory_), "root_directory should exist");
        }

        bool AddResource(const FileVaultResourceProviderPtr &resource_provider) override {
            const auto name = resource_provider->GetResourceName();
            if (resources_.contains(name)) {
                LOG_WARN("duplicate resource in filevault. name={}, root={}", name, root_directory_);
                return false;
            }
            std::lock_guard lock(write_mutex_);
            resources_[name] = resource_provider;
            return true;
        }

        std::future<FileVaultResourceEntry> GetResource(const std::string &named_resource) override {
            return std::async(std::launch::async, [&] {
                assert_true(resources_.contains(named_resource),
                            fmt::format("Resource {} should exist in vault", named_resource));

                // lock and get resource provider
                std::lock_guard lock(resource_mutexes_[named_resource]);
                const auto resource_provider = resources_.at(named_resource);

                // get resource paths
                const auto [data_path, json_path] = GetFilePaths_(resource_provider->GetResourceName());

                if (!std::filesystem::exists(data_path) || !std::filesystem::exists(json_path)) {
                    LOG_DEBUG("json and data file not found for resource. fetch process will be triggered. name={}",
                              named_resource);
                    return details::fetch_resource(resource_provider, data_path, json_path);
                }

                // all done
                LOG_DEBUG("local data files are present for resource {}", named_resource);
                return details::build_entry_from_json(data_path, json_path);
            });
        }

        std::future<bool> CheckResource(const std::string &named_resource) override {
            return std::async(std::launch::async, [&] {
                std::lock_guard lock(write_mutex_);
                const auto [p1,p2] = GetFilePaths_(named_resource);
                return resources_.contains(named_resource) && std::filesystem::exists(p1) && std::filesystem::exists(p2);
            });
        }

        std::future<void> DeleteResource(const std::string &named_resource) override {
            return std::async(std::launch::async, [&] {
                if (CheckResource(named_resource).get()) {
                    std::lock_guard lock(write_mutex_);
                    const auto [p1,p2] = GetFilePaths_(named_resource);
                    std::filesystem::remove(p1);
                    std::filesystem::remove(p2);
                    resources_.erase(named_resource);
                }
            });
        }

    private:
        [[nodiscard]] std::pair<std::string, std::string> GetFilePaths_(const std::string &named_resource) const {
            const std::string file_id = HashUtils::HashForString<MD5>(named_resource);
            return {
                root_directory_ / (file_id + ".dat"),
                root_directory_ / (file_id + ".json")
            };
        }

    };

    /**
     * Create a shared instance of FileSystemFileVault
     * @param root root folder for vault
     * @return
     */
    static FileVaultPtr CreateFilesystemFileVault(const std::filesystem::path& root) {
        if(!std::filesystem::exists(root)) {
            std::filesystem::create_directories(root);
        }
        return std::make_shared<FileSystemFileVault>(root);
    }


    /**
    *  Default FileSystemFileVault instance.
    */
    static auto DEFAULT_FILE_VAULT = CreateFilesystemFileVault(SystemUtils::GetHomeDirectory() / ".instinct");


}


#endif //FILESYSTEMFILEVAULT_HPP
