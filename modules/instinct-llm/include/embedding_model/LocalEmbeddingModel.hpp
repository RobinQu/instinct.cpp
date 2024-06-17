//
// Created by RobinQu on 2024/6/17.
//

#ifndef LOCALEMBEDDINGMODEL_HPP
#define LOCALEMBEDDINGMODEL_HPP


#include "LLMGlobals.hpp"
#include "models.hpp"
#include "model_factory.hpp"
#include "tokenizer.hpp"
#include "model/IEmbeddingModel.hpp"
#include "tools/file_vault/FileSystemFileVault.hpp"
#include "tools/file_vault/IFileVault.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_TRANSFORMER_NS;

    class LocalEmbeddingModel final: public IEmbeddingModel {
        transformer::tokenizer::TokenizerPtr tokenizer_;
        models::ModelPtr model_;
        size_t dimension_;
    public:
        explicit LocalEmbeddingModel(const std::filesystem::path& model_file_path) {
            std::tie(model_, tokenizer_) = ModelFactory::GetInstance().load(model_file_path);
            dimension_ = model_->get_text_embedding_dim();
        }

        std::vector<Embedding> EmbedDocuments(const std::vector<std::string> &texts) override {
            std::vector<Embedding> output;
            output.reserve(texts.size());
            for(int i=0;i<texts.size();++i) {
                output[i] = EmbedQuery(texts[i]);
            }
            return output;
        }

        Embedding EmbedQuery(const std::string &text) override {
            constexpr GenerationConfig config {};
            Embedding embedding;
            model_->text_embedding(config, tokenizer_->encode(text), embedding);
            return embedding;
        }

        size_t GetDimension() override {
            return dimension_;
        }
    };

    static void PreloadEmbeddingModelFiles(const FileVaultPtr& file_vault = DEFAULT_FILE_VAULT) {
        if(const auto resource_name = "model_bins/" + to_file_name(ModelType::BGE_M3_EMBEDDING); !file_vault->CheckResource(resource_name).get()) {
            FetchHttpGetResourceToFileVault(
                file_vault,
                resource_name,
                "https://huggingface.co/robinqu/baai-bge-m3-guff/resolve/main/bge-m3e.bin?download=true",
                {.algorithm = kSHA256, .expected_value = "982c136f3804b008fe5a2e7a46fafcc5624277aa0c856bb8ce7bfc521fd4bdfb"}
            ).wait();
        }
    }

    static EmbeddingsPtr CreateLocalRankingModel(const ModelType model_type, const FileVaultPtr& file_vault = DEFAULT_FILE_VAULT) {
        static std::mutex FILE_MUTEX;
        std::lock_guard file_lock {FILE_MUTEX};
        PreloadEmbeddingModelFiles(file_vault);
        const auto resource_name = "model_bins/" + to_file_name(model_type);
        const auto entry = file_vault->GetResource(resource_name).get();
        return std::make_shared<LocalEmbeddingModel>(entry.local_path);
    }

}


#endif //LOCALEMBEDDINGMODEL_HPP
