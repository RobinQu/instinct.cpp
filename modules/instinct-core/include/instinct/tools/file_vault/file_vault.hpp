//
// Created by RobinQu on 2024/4/15.
//

#ifndef FILECACHES_HPP
#define FILECACHES_HPP
#include <filesystem>
#include <ostream>

#include <instinct/core_global.hpp>
#include <instinct/tools/http/http_client.hpp>


namespace INSTINCT_CORE_NS {


    using FileVaultResourceEntryMetadata = std::unordered_map<std::string, std::string>;

    enum ChecksumAlgorithm {
        kNoChecksum,
        kSHA1,
        kSHA256,
        kMD5
    };

    struct ChecksumRequest {
        ChecksumAlgorithm algorithm;
        std::string expected_value;
    };

    static std::string to_string(const ChecksumAlgorithm algorithm) {
        if (algorithm == kSHA1) return "SHA1";
        if (algorithm == kSHA256) return "SHA256";
        if (algorithm == kMD5) return "MD5";
        return "NoChecksum";
    }

    static ChecksumAlgorithm from_string(const std::string& str) {
        if (str == "SHA1") return kSHA1;
        if (str == "SHA256") return kSHA256;
        if (str == "MD5") return kMD5;
        return kNoChecksum;
    }


    class IFileVaultResourceProvider {
    public:
        IFileVaultResourceProvider()=default;
        virtual ~IFileVaultResourceProvider()=default;
        IFileVaultResourceProvider(IFileVaultResourceProvider&&)=delete;
        IFileVaultResourceProvider(const IFileVaultResourceProvider&)=delete;
        [[nodiscard]] virtual const ChecksumRequest& GetChecksum() const = 0;
        virtual FileVaultResourceEntryMetadata& GetMetadata() = 0;
        [[nodiscard]] virtual const std::string& GetResourceName() const = 0;
        virtual std::future<void> Persist(std::ostream& ostream) = 0;
    };

    using FileVaultResourceProviderPtr = std::shared_ptr<IFileVaultResourceProvider>;

    struct FileVaultResourceEntry {
        std::string name;
        FileVaultResourceEntryMetadata metadata;
        std::filesystem::path local_path;
        ChecksumAlgorithm checksum_algorithm;
        std::string checksum_value;
    };

    /**
     * Managed file library interface
     */
    class IFileVault {
    public:
        IFileVault()=default;
        virtual ~IFileVault()=default;
        IFileVault(IFileVault &&)=delete;
        IFileVault(const IFileVault&)=delete;

        /**
         * Register a `ResourceProvider`. Implementations may choose whether fetch should be done immediately or later.
         * @param resource_provider
         * @return
         */
        virtual bool AddResource(const FileVaultResourceProviderPtr& resource_provider) = 0;


        /**
         * Check presence of given resource
         * @param named_resource
         * @return
         */
        virtual std::future<bool> CheckResource(const std::string& named_resource) = 0;

        virtual std::future<void> DeleteResource(const std::string& named_resource) = 0;

        /**
         * Get local resource path for given resource. Implementation may choose to cache.
         * @param named_resource
         * @return
         * @throw InstinctException If there is no given resource or error occurs during fetching content, exception will be thrown when you call `get` on returned future.
         */
        virtual std::future<FileVaultResourceEntry> GetResource(const std::string& named_resource) = 0;

    };

    using FileVaultPtr = std::shared_ptr<IFileVault>;


}


#endif //FILECACHES_HPP
