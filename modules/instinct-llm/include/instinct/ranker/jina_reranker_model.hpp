//
// Created by RobinQu on 2024/6/29.
//

#ifndef JINA_RERANKER_MODEL_HPP
#define JINA_RERANKER_MODEL_HPP
#include <instinct/jina_api.pb.h>
#include <instinct/ranker/base_ranking_model.hpp>

#include "instinct/tools/http_rest_client.hpp"

namespace INSTINCT_LLM_NS {

    static const Endpoint JINA_DEFAULT_ENDPOINT = {
        .host = "api.jina.ai",
        .port = 443,
        .protocol = kHTTPS
    };

    class RemoteRerankerModel: public BaseRankingModel {
    };

    struct JinaConfiguration {
        Endpoint endpoint {};
        std::string api_key;
        std::string model_name;
    };


    class JinaRerankerModel final: public RemoteRerankerModel {
        HttpRestClient client_;
        JinaConfiguration configuration_;
    public:
        explicit JinaRerankerModel(const JinaConfiguration& configuration)
            : client_(configuration.endpoint), configuration_(configuration) {
            assert_not_blank(configuration_.api_key, "should provide api key for jina.ai");
            client_.GetDefaultHeaders().emplace("Authorization", fmt::format("Bearer {}", configuration_.api_key));
        }

        float GetRankingScore(const std::string &query, const std::string &doc) override {
            Document document;
            document.set_text(doc);
            const auto result = RerankDocuments({document}, query, 1);
            assert_non_empty_range(result);
            return result.front().second;
        }

        /**
        * https://api.jina.ai/redoc#tag/rerank
        */
        std::vector<IdxWithScore> RerankDocuments(const std::vector<Document> &docs, const std::string &query, const int top_n) override {
            JinaRerankRequest request;
            const int n = static_cast<int>(docs.size());
            request.set_top_n(top_n > 0 ? std::min(n, top_n) : n);
            request.set_return_documents(false);
            request.set_model(configuration_.model_name);
            request.set_query(query);
            for(const auto& doc: docs) {
                request.add_documents(doc.text());
            }
            const auto response = client_.PostObject<JinaRerankRequest, JinaRerankResponse>("/v1/rerank", request);
            std::vector<IdxWithScore> result;
            for(const auto& item: response.results()) {
                if (item.index() < docs.size()) {
                    result.emplace_back(item.index(), item.relevance_score());
                }
            }
            // sort again
            std::ranges::sort(result, std::ranges::greater{}, &IdxWithScore::second);
            return result;
        }
    };

    static void LoadJinaRerankerConfiguration(JinaConfiguration& configuration) {
        if (StringUtils::IsBlankString(configuration.api_key)) {
            configuration.api_key = SystemUtils::GetEnv("JINA_API_KEY");
        }
        if (StringUtils::IsBlankString(configuration.model_name)) {
            configuration.model_name = "jina-reranker-v2-base-multilingual";
        }
        if (StringUtils::IsBlankString(configuration.endpoint.host)) {
            configuration.endpoint.host =SystemUtils::GetEnv("JINA_HOST", JINA_DEFAULT_ENDPOINT.host);
        }
        if (configuration.endpoint.port == 0) {
            configuration.endpoint.port = SystemUtils::GetIntEnv("JINA_PORT", JINA_DEFAULT_ENDPOINT.port);
        }
        if (configuration.endpoint.protocol == kUnspecifiedProtocol) {
            configuration.endpoint.protocol = StringUtils::ToLower(SystemUtils::GetEnv("JINA_PROTOCOL", "https"))  == "https" ? kHTTPS : kHTTP;
        }
    }

    static RankingModelPtr CreateJinaRerankerModel(JinaConfiguration configuration = {}) {
        LoadJinaRerankerConfiguration(configuration);
        return std::make_shared<JinaRerankerModel>(configuration);
    }
}


#endif //JINA_RERANKER_MODEL_HPP
