//
// Created by RobinQu on 2024/4/18.
//

#ifndef BASEFILEVAULTRESOURCEPROVIDER_HPP
#define BASEFILEVAULTRESOURCEPROVIDER_HPP

#include <utility>

#include <instinct/tools/file_vault/base_file_vault_resource_provider.hpp>

#include "file_vault.hpp"

namespace INSTINCT_CORE_NS {
    class BaseFileVaultResourceProvider: public IFileVaultResourceProvider {
        /**
         * Resource name
         */
        std::string resource_name;

        /**
         * meatdata
         */
        std::unordered_map<std::string,std::string> metadata;

        ChecksumRequest checksum_;

    public:

        [[nodiscard]] const std::string& GetResourceName() const override {
            return resource_name;
        }

        FileVaultResourceEntryMetadata& GetMetadata() override {
            return metadata;
        }

        explicit BaseFileVaultResourceProvider(std::string resource_name,
            ChecksumRequest checksum,
            FileVaultResourceEntryMetadata metadata = {})
            : resource_name(std::move(resource_name)),
              metadata(std::move(metadata)),
                checksum_(std::move(checksum)){
        }


        std::future<void> Persist(std::ostream &ostream) override {
            return std::async(std::launch::async, [&]() {
                ostream.clear();
                this->Write(ostream);
            });
        }

        [[nodiscard]] const ChecksumRequest & GetChecksum() const override {
            return checksum_;
        }

    private:

        /**
         * A function for writing down resource in a sync manaer
         * @param ostream a writable stream for resource content
         */
        virtual void Write(std::ostream &ostream) = 0;
    };


}

#endif //BASEFILEVAULTRESOURCEPROVIDER_HPP
