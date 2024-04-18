//
// Created by RobinQu on 2024/4/18.
//
#include <gtest/gtest.h>

#include "tools/IOUtils.hpp"
#include "tools/file_vault/FileSystemFileVault.hpp"


namespace INSTINCT_CORE_NS {

    class FileSystemFileVaultTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
            file_vault_ = CreateFilesystemFileVault(std::filesystem::temp_directory_path());
        }

        FileVaultPtr file_vault_;
    };

    TEST_F(FileSystemFileVaultTest, FetchWithChecksum) {
        const auto payload = StringUtils::GenerateUUIDString();
        const auto payload_sha1 = HashUtils::HashForString<SHA1>(payload);
        const auto payload_sha256 = HashUtils::HashForString<SHA256>(payload);
        const auto payload_md5 = HashUtils::HashForString<MD5>(payload);
        const auto payload_base64 = CodecUtils::EncodeBase64(payload);

        const auto r1_name = StringUtils::GenerateUUIDString();
        const auto entry1 = FetchHttpGetResourceToFileVault(file_vault_, r1_name, "https://httpbin.org/base64/"  + payload_base64, {.algorithm = kMD5, .expected_value = payload_md5}).get();
        ASSERT_EQ(IOUtils::ReadString(entry1.local_path), payload);

        const auto r2_name = StringUtils::GenerateUUIDString();
        const auto entry2 = FetchHttpGetResourceToFileVault(file_vault_, r2_name, "https://httpbin.org/base64/"  + payload_base64, {.algorithm = kSHA256, .expected_value = payload_sha256}).get();
        ASSERT_EQ(IOUtils::ReadString(entry2.local_path), payload);

        const auto r3_name = StringUtils::GenerateUUIDString();
        const auto entry3 = FetchHttpGetResourceToFileVault(file_vault_, r3_name, "https://httpbin.org/base64/"  + payload_base64, {.algorithm = kSHA1, .expected_value = payload_sha1}).get();
        ASSERT_EQ(IOUtils::ReadString(entry3.local_path), payload);
    }

    TEST_F(FileSystemFileVaultTest, ThrowWithUnmatchChecksum) {
        ASSERT_THROW({
            FetchHttpGetResourceToFileVault(file_vault_, StringUtils::GenerateUUIDString(), "https://httpbin.org/get", {.algorithm = kMD5, .expected_value = "123"}).get();
        }, InstinctException);
    }

    TEST_F(FileSystemFileVaultTest, TestUtilityFunction) {
        const auto name1 = StringUtils::GenerateUUIDString();
        FetchHttpGetResourceToFileVault(file_vault_, name1, "https://httpbin.org/get", {.algorithm = kNoChecksum}).get();
        ASSERT_TRUE(file_vault_->CheckResourcePresence(name1).get());

        file_vault_->DeleteResource(name1).get();
        ASSERT_FALSE(file_vault_->CheckResourcePresence(name1).get());
    }

}