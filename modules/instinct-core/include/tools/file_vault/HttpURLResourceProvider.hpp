//
// Created by RobinQu on 2024/4/18.
//
#ifndef IHTTPURLRESOURCEPROVIDER_HPP
#define IHTTPURLRESOURCEPROVIDER_HPP


#include "BaseFileVaultResourceProvider.hpp"
#include "tools/http/CURLHttpClient.hpp"
#include "tools/http/IHttpClient.hpp"

namespace INSTINCT_CORE_NS {

    class HttpURLResourceProvider final: public BaseFileVaultResourceProvider {
        HttpClientPtr client_;
        HttpRequest call_;

    public:
        HttpURLResourceProvider(
            const std::string &resource_name,
            HttpRequest call,
            const ChecksumRequest& request ={.algorithm = kNoChecksum},
            const FileVaultResourceEntryMetadata &metadata = {},
            HttpClientPtr client = nullptr)
            : BaseFileVaultResourceProvider(resource_name, request, metadata),
              client_(std::move(client)),
              call_(std::move(call)) {
            if (!client_) {
                client_ = CreateCURLHttpClient();
            }
            HttpUtils::AssertHttpRequest(call_);
        }

        HttpURLResourceProvider(
            const std::string &resource_name,
            const std::string &request_line,
            const ChecksumRequest& request ={.algorithm = kNoChecksum},
            const std::unordered_map<std::string, std::string> &metadata = {},
            const HttpClientPtr& client = nullptr
            ): HttpURLResourceProvider(resource_name, HttpUtils::CreateRequest(request_line), request, metadata, client)  {
        }

    private:
        void Write(std::ostream &output_stream) override {
            // LOG_INFO("Writing to ostream");
            const auto [headers, status_code] = client_->ExecuteWithCallback(call_, [&](const std::string& buf) {
                output_stream.write(buf.data(), buf.size());
                return true;
            });
            assert_true(status_code >= 200 && status_code < 400, fmt::format("Status code {} for URL {}", status_code, HttpUtils::CreateUrlString(call_)));
            // add response headers to metadata
            GetMetadata().insert(headers.begin(), headers.end());
        }
    };

    static FileVaultResourceProviderPtr CreateHttpURLResourceProvider(const std::string& resource_name, const std::string& request_line) {
        return std::make_shared<HttpURLResourceProvider>(resource_name, request_line);
    }



}

#endif