//
// Created by RobinQu on 2024/4/18.
//

#ifndef IFILEVAULTRESOURCEPROVIDER_HPP
#define IFILEVAULTRESOURCEPROVIDER_HPP

#include "CoreGlobals.hpp"

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
        [[nodiscard]] virtual [[nodiscard]] const ChecksumRequest& GetChecksum() const = 0;
        virtual FileVaultResourceEntryMetadata& GetMetadata() = 0;
        [[nodiscard]] virtual const std::string& GetResourceName() const = 0;
        virtual std::future<void> Persist(std::ostream& ostream) = 0;
    };

    using FileVaultResourceProviderPtr = std::shared_ptr<IFileVaultResourceProvider>;
}
#endif //IFILEVAULTRESOURCEPROVIDER_HPP
