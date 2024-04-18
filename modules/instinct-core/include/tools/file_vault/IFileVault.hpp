//
// Created by RobinQu on 2024/4/15.
//

#ifndef FILECACHES_HPP
#define FILECACHES_HPP
#include <filesystem>
#include <ostream>

#include "CoreGlobals.hpp"
#include "HttpURLResourceProvider.hpp"
#include "IFileVaultResourceProvider.hpp"
#include "tools/http/IHttpClient.hpp"


namespace INSTINCT_CORE_NS {

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
        virtual std::future<bool> CheckResourcePresence(const std::string& named_resource) = 0;

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

    /**
     * Add a remote resource located at given url string and fetch immediately
     * @param file_vault
     * @param resource_name
     * @param url_string http or https protocol string
     * @param checksum chcksum info for given file
     * @return
     */
    static std::future<FileVaultResourceEntry> FetchHttpGetResourceToFileVault(const FileVaultPtr& file_vault, const std::string& resource_name, const std::string& url_string, const ChecksumRequest& checksum) {
        const auto resource = std::make_shared<HttpURLResourceProvider>(resource_name, "GET " + url_string, checksum);
        assert_true(file_vault->AddResource(resource), "should have added the resource");
        return file_vault->GetResource(resource_name);
    }


}


#endif //FILECACHES_HPP
