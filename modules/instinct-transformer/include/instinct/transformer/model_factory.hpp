//
// Created by RobinQu on 2024/5/26.
//

#ifndef MODEL_FACTORY_HPP
#define MODEL_FACTORY_HPP

#include <instinct/transformer_globals.hpp>
#include <instinct/transformer/models.hpp>
#include <instinct/transformer/tokenizer.hpp>
#include <instinct/transformer/config.hpp>
#include <instinct/transformer/models/bge_ranker.hpp>

namespace INSTINCT_TRANSFORMER_NS {
    using namespace INSTINCT_TRANSFORMER_NS::tokenizer;
    using namespace INSTINCT_TRANSFORMER_NS::models;

    static std::string to_file_name(const ModelType model_type) {
        if (model_type == ModelType::BGE_M3_RERANKER) {
            return "bge-reranker-v2-m3.bin";
        }
        if (model_type == ModelType::BGE_M3_EMBEDDING) {
            return "bge-m3e.bin";
        }

        throw std::runtime_error("unknown model type");
    }


    /**
     * A factory class that manages lifecycle of model instances
     */
    class ModelFactory {
        /**
        * hold all loaded models
        */
        std::map<std::string, std::shared_ptr<ModelLoader>> model_loaders_;
        std::mutex mutex_;

    public:
        static ModelFactory& GetInstance() {
            static ModelFactory INSTANCE_;
            return INSTANCE_;
        }

        /**
         * Load model by weight file path. Same instance will be returned for single model path.
         * @param model_path
         * @return
         */
        std::pair<ModelPtr, TokenizerPtr> load(const std::string& model_path) {
            std::shared_ptr<ModelLoader> loader = nullptr;
            { // sequential access to model_loaders_
                std::lock_guard loader_lock {mutex_};
                if(model_loaders_.contains(model_path)) {
                    loader = model_loaders_.at(model_path);
                } else {
                    loader = std::make_shared<ModelLoader>(model_path);
                    model_loaders_.emplace(model_path, loader);
                }
            }

            // read headers
            loader->seek(0, SEEK_SET);
            const std::string magic = loader->read_string(4);
            GGML_ASSERT(magic == "ggml");
            const auto model_type = loader->read_basic<int>();
            const auto version = loader->read_basic<int>();

            switch (model_type) {
                case ModelType::BGE_M3_RERANKER: {
                    GGML_ASSERT(version == 1);
                    return load_<bge::ranker::Tokenizer, bge::ranker::BGERerankerModel, bge::ranker::Config>(*loader);
                }
                case ModelType::BGE_M3_EMBEDDING: {
                    GGML_ASSERT(version == 1);
                    return load_<bge::embedding::Tokenizer, bge::embedding::BGEEmbeddingModel, bge::embedding::Config>(*loader);
                }
                default:
                    // TODO throw exception
                    GGML_ASSERT(false);
            }
        }

    private:
        template<typename Tokenizer, typename Model, typename Config>
        requires std::derived_from<Tokenizer, BaseTokenizer> && std::derived_from<Model, BaseModel> && std::derived_from<Config, BaseConfig>
        std::pair<ModelPtr, TokenizerPtr> load_(ModelLoader& loader) {
            // read config
            if (0 == loader.offset_config)
                loader.offset_config = loader.tell();
            else
                loader.seek(loader.offset_config, SEEK_SET);

            // load config
            auto config = loader.read_basic<Config>();

            // load tokenizer
            loader.offset_tokenizer = loader.tell();
            loader.seek(loader.offset_tokenizer, SEEK_SET);
            auto tokenizer = std::make_shared<Tokenizer>(config);
            auto proto_size = tokenizer->load(loader.data + loader.tell(), config.vocab_size);
            loader.seek(proto_size, SEEK_CUR);

            // load tensors
            if (0 == loader.offset_tensors)
            {
                loader.offset_tensors = loader.tell();
                loader.load_all_tensors();
            }
            loader.seek(loader.offset_tensors, SEEK_SET);

            // load model
            auto model = std::make_shared<Model>(config);
            model->load(loader);

            // return
            return {model, tokenizer};
        }
    };

}


#endif //MODEL_FACTORY_HPP
