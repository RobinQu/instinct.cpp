//
// Created by root on 5/24/24.
//

#ifndef CXX_TEST_BGE_M3_HPP
#define CXX_TEST_BGE_M3_HPP
#include <ggml.h>
#include <vector>

#include <instinct/transformer/config.hpp>
#include <instinct/transformer/layers.hpp>
#include <instinct/transformer/ops.hpp>
#include <instinct/transformer/models.hpp>
#include <instinct/transformer/tokenizer.hpp>
#include <instinct/transformer/models/bge_embedding.hpp>

namespace INSTINCT_TRANSFORMER_NS::models::bge::ranker {
    using namespace INSTINCT_TRANSFORMER_NS::layers;
    using namespace INSTINCT_TRANSFORMER_NS::ops;
    using namespace INSTINCT_TRANSFORMER_NS::models;
    using namespace INSTINCT_TRANSFORMER_NS::tokenizer;

    struct Config: public embedding::Config {};

    class Tokenizer final: public embedding::Tokenizer {
    public:
        explicit Tokenizer(const Config &config)
            : embedding::Tokenizer(config) {
        }

        void encode_qa(const std::string &q, const std::string &a, std::vector<int> &ids) const override {
            const int max_length = this->max_length - 2;

            std::vector<int> ids_q;
            std::vector<int> ids_a;
            BaseTokenizer::encode(q, ids_q);
            BaseTokenizer::encode(a, ids_a);

            int total = (int)ids_q.size() + (int)ids_a.size();

            // this is bad
            if (total > max_length - 4)
            {
                int remain = max_length - 4 - (int)ids_q.size();
                GGML_ASSERT(remain > 0);

                ids_a.resize(remain);
            }

            ids.push_back(bos_token_id);
            ids.insert(std::end(ids), std::begin(ids_q), std::end(ids_q));
            ids.push_back(eos_token_id);

            ids.push_back(bos_token_id);
            ids.insert(std::end(ids), std::begin(ids_a), std::end(ids_a));
            ids.push_back(eos_token_id);
        }
    };

    class BGERerankerModel final: public BaseGenerationModel<XLMRoberta<RobertaClassificationHead>> {
    public:
        static constexpr size_t MEM_SIZE = 812ull * 1024 * 1024;
        static constexpr size_t SCRATCH_SIZE = 544ull * 1024 * 1024;

        explicit BGERerankerModel(const Config& config, const size_t mem_size = MEM_SIZE, const size_t scratch_size = SCRATCH_SIZE):
            BaseGenerationModel(BGE_M3_RERANKER, Ranker, config, mem_size, scratch_size),
                w_ctx_(
                       {
                           ggml_init({.mem_size = ((9 + config.num_hidden_layers * 19) * (GGML_TENSOR_SIZE + GGML_OBJECT_SIZE)), .mem_buffer = nullptr, .no_alloc = true}),
                           config.dtype
                       }
                ),
                transformer_(&w_ctx_, config)
        {
        }

        XLMRoberta<RobertaClassificationHead>& get_transformer() override {
            return transformer_;
        }

        void load(ModelLoader &loader) override {
            loader.read_tensor("embeddings.word_embeddings.weight",         transformer_.word_embeddings.word_weight);
            loader.read_tensor("embeddings.position_embeddings.weight",     transformer_.word_embeddings.position_weight);
            loader.read_tensor("embeddings.LayerNorm.weight",               transformer_.word_embeddings.ln.weight);
            loader.read_tensor("embeddings.LayerNorm.bias",                 transformer_.word_embeddings.ln.bias);

            for (int i = 0; i < config_.num_hidden_layers; i++)
            {
                std::string layer_prefix = "encoder.layer." + std::to_string(layer_ids[i]) + '.';
                loader.read_tensor(layer_prefix + "attention.self.query.weight",    transformer_.layers[i].attention.q_proj.weight);
                loader.read_tensor(layer_prefix + "attention.self.query.bias",      transformer_.layers[i].attention.q_proj.bias);
                loader.read_tensor(layer_prefix + "attention.self.key.weight",      transformer_.layers[i].attention.k_proj.weight);
                loader.read_tensor(layer_prefix + "attention.self.key.bias",        transformer_.layers[i].attention.k_proj.bias);
                loader.read_tensor(layer_prefix + "attention.self.value.weight",    transformer_.layers[i].attention.v_proj.weight);
                loader.read_tensor(layer_prefix + "attention.self.value.bias",      transformer_.layers[i].attention.v_proj.bias);
                loader.read_tensor(layer_prefix + "attention.output.dense.weight",  transformer_.layers[i].attention.o_proj.weight);
                loader.read_tensor(layer_prefix + "attention.output.dense.bias",    transformer_.layers[i].attention.o_proj.bias);
                loader.read_tensor(layer_prefix + "attention.output.LayerNorm.weight",    transformer_.layers[i].post_attention_layer_norm.weight);
                loader.read_tensor(layer_prefix + "attention.output.LayerNorm.bias",      transformer_.layers[i].post_attention_layer_norm.bias);

                loader.read_tensor(layer_prefix + "intermediate.dense.weight",  transformer_.layers[i].mlp.intermediate.weight);
                loader.read_tensor(layer_prefix + "intermediate.dense.bias",    transformer_.layers[i].mlp.intermediate.bias);
                loader.read_tensor(layer_prefix + "output.dense.weight",        transformer_.layers[i].mlp.output.dense.weight);
                loader.read_tensor(layer_prefix + "output.dense.bias",          transformer_.layers[i].mlp.output.dense.bias);

                loader.read_tensor(layer_prefix + "output.LayerNorm.weight",    transformer_.layers[i].mlp.output.norm.weight);
                loader.read_tensor(layer_prefix + "output.LayerNorm.bias",      transformer_.layers[i].mlp.output.norm.bias);
            }

            loader.read_tensor("classifier.dense.weight",       transformer_.final.dense.weight);
            loader.read_tensor("classifier.dense.bias",         transformer_.final.dense.bias);
            loader.read_tensor("classifier.out_proj.weight",    transformer_.final.out_proj.weight);
            loader.read_tensor("classifier.out_proj.bias",      transformer_.final.out_proj.bias);

            GGML_ASSERT(ggml_used_mem(w_ctx_.g_ctx) == ggml_get_mem_size(w_ctx_.g_ctx));
        }
    private:
        InitContext w_ctx_; // weight context
        XLMRoberta<RobertaClassificationHead> transformer_;

    };

}





#endif //CXX_TEST_BGE_M3_HPP
