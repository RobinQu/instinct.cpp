//
// Created by RobinQu on 2024/5/26.
//

#ifndef MODEL_FACTORY_HPP
#define MODEL_FACTORY_HPP

#include "./globals.hpp"
#include "./model.hpp"
#include "./tokenizer.hpp"
#include "./config.hpp"
#include "models/bge_m3.hpp"

namespace INSTINCT_TRANSFORMER_NS {
    using namespace INSTINCT_TRANSFORMER_NS::tokenizer;
    using namespace INSTINCT_TRANSFORMER_NS::models;

    class ModelFactory {
    public:
        std::pair<ModelPtr, TokenizerPtr> load(const std::string& model_path) {
            ModelLoader loader {model_path};
            // read headers
            loader.seek(0, SEEK_SET);
            const std::string magic = loader.read_string(4);
            GGML_ASSERT(magic == "ggml");
            const auto model_type = loader.read_basic<int>();
            const auto version = loader.read_basic<int>();

            switch (model_type) {
                case ModelType::BGE_M3_RERANKER: {
                    GGML_ASSERT(version == 1);
                    return load_<bge::Tokenizer, bge::BGEM3RerankerModel, bge::Config>(loader);
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
            auto config = loader.read_basic<BaseConfig>();

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
            model.Load(loader);

            // return
            return  {tokenizer, model};
        }
    };

}


#endif //MODEL_FACTORY_HPP
