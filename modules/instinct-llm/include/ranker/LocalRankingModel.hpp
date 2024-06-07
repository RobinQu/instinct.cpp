//
// Created by RobinQu on 2024/5/26.
//

#ifndef LOCALRANKINGMODEL_HPP
#define LOCALRANKINGMODEL_HPP

#include "./BaseRankingModel.hpp"
#include "model.hpp"
#include "model_factory.hpp"
#include "tools/file_vault/FileSystemFileVault.hpp"

namespace INSTINCT_LLM_NS {
    using namespace  INSTINCT_TRANSFORMER_NS;

    /**
     * This class uses models in `instinct-transformer` module to calculate ranking score
     */
    class LocalRankingModel final: public BaseRankingModel {
        transformer::tokenizer::TokenizerPtr tokenizer_;
        ModelPtr model_;
    public:
        explicit LocalRankingModel(const ModelType model_type, const FileVaultPtr& file_vault) {
            const auto resource_name = "model_bins/" + to_file_name(model_type);
            const auto entry = file_vault->GetResource(resource_name).get();
            std::tie(model_, tokenizer_) = ModelFactory::GetInstance().load(entry.local_path);
        }

        float GetRankingScore(const std::string &query, const std::string &doc) override {
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
                "https://huggingface.co/robinqu/baai-bge-m3-guff/resolve/main/bge-reranker-v2-m3.bin?download=true",
                {.algorithm = kSHA256, .expected_value = "b3e05dbe06c0aa52fd974d9c9dedbc51292b81f2f285d56113c060a0931a7f0f"}
            ).wait();
        }
    }

    static RankingModelPtr CreateLocalRankingModel(const ModelType model_type, const FileVaultPtr& file_vault = DEFAULT_FILE_VAULT) {
        PreloadRankingModelFiles(file_vault);
        return std::make_shared<LocalRankingModel>(model_type, file_vault);
    }

}

#endif //LOCALRANKINGMODEL_HPP
