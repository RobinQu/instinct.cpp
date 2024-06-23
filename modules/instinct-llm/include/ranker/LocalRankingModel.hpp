//
// Created by RobinQu on 2024/5/26.
//

#ifndef LOCALRANKINGMODEL_HPP
#define LOCALRANKINGMODEL_HPP

#include "./BaseRankingModel.hpp"
#include "models.hpp"
#include "model_factory.hpp"
#include "tools/file_vault/FileSystemFileVault.hpp"

namespace INSTINCT_LLM_NS {
    using namespace  INSTINCT_TRANSFORMER_NS;

    /**
     * This class uses models in `instinct-transformer` module
     */
    class LocalRankingModel final: public BaseRankingModel {
        transformer::tokenizer::TokenizerPtr tokenizer_;
        ModelPtr model_;
        std::mutex run_mutex_;
    public:
        explicit LocalRankingModel(const std::filesystem::path& model_file_path) {
            std::tie(model_, tokenizer_) = ModelFactory::GetInstance().load(model_file_path);
        }

        float GetRankingScore(const std::string &query, const std::string &doc) override {
            trace_span span {"GetRankingScore"};
            // TODO remove this lock by using multi-instance or batching.
            std::lock_guard guard {run_mutex_};
            constexpr GenerationConfig config {};
            std::vector<int> ids;
            this->tokenizer_->encode_qa(query, doc, ids);
            return this->model_->qa_rank(config, ids);
        }
    };

    static void PreloadRankingModelFiles(const FileVaultPtr& file_vault = DEFAULT_FILE_VAULT) {
        if(const auto resource_name = "model_bins/" + to_file_name(ModelType::BGE_M3_RERANKER); !file_vault->CheckResource(resource_name).get()) {
            FetchHttpGetResourceToFileVault(
                file_vault,
                resource_name,
                "https://huggingface.co/robinqu/baai-bge-m3-guff/resolve/main/bge-reranker-m3-q4_1.bin?download=true",
                {.algorithm = kSHA256, .expected_value = "2b0d1b8f41b8cfd6bc18aa90dd89671ba9aeb0d8121688f8b6a044634761de0c"}
            ).wait();
        }
    }

    static RankingModelPtr CreateLocalRankingModel(const ModelType model_type, const FileVaultPtr& file_vault = DEFAULT_FILE_VAULT) {
        static std::mutex FILE_MUTEX;
        std::lock_guard file_lock {FILE_MUTEX};
        PreloadRankingModelFiles(file_vault);
        const auto resource_name = "model_bins/" + to_file_name(model_type);
        const auto entry = file_vault->GetResource(resource_name).get();
        return std::make_shared<LocalRankingModel>(entry.local_path);
    }

}

#endif //LOCALRANKINGMODEL_HPP
