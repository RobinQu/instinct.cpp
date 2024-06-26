//
// Created by RobinQu on 2024/6/17.
//

#ifndef BGE_EMBEDDING_HPP
#define BGE_EMBEDDING_HPP
#include <instinct/./config.hpp>
#include <instinct/./layers.hpp>
#include <instinct/./ops.hpp>
#include <instinct/./models.hpp>
#include <instinct/./tokenizer.hpp>


namespace INSTINCT_TRANSFORMER_NS::models::bge::embedding {
    using namespace INSTINCT_TRANSFORMER_NS::layers;
    using namespace INSTINCT_TRANSFORMER_NS::ops;
    using namespace INSTINCT_TRANSFORMER_NS::models;
    using namespace INSTINCT_TRANSFORMER_NS::tokenizer;

    struct Config: public BaseConfig {};

    class Tokenizer: public BaseTokenizer {
    public:
        explicit Tokenizer(const Config &config) : BaseTokenizer(config) {}

        size_t load(const char *buffer, const int n_vocab) override {
            tp = new UnigramProcessor(eos_token_id + 1);
            tp->RegisterPreprocessor(new TextPrepNewlineToSpaces());
            tp->RegisterPreprocessor(new TextPrepDeleteMultiSpaces());
            tp->RegisterPreprocessor(new TextPrepAddLeadingSpace());
            const auto size = tp->Load(buffer, n_vocab);
            return size;
        }

        void encode(const std::string &text, std::vector<int> &ids) const override {
            // position embedding offset = 2
            encode(text, ids, true, true, max_length - 2);
        }


    protected:
        void encode(const std::string &text, std::vector<int> &ids, bool add_bos, bool add_eos, int max_length) const {
            if (add_bos) max_length--;
            if (add_eos) max_length--;

            if (add_bos)
                ids.push_back(bos_token_id);
            size_t start = ids.size();
            BaseTokenizer::encode(text, ids);
            size_t length = ids.size() - start;
            if ((max_length > 0) && ((int) length > max_length))
                ids.resize(start + max_length);

            if (add_eos)
                ids.push_back(eos_token_id);
        }
    };

    class BGEEmbeddingModel final: public BaseGenerationModel<XLMRoberta<BCEFinalNorm>> {
    public:
        static constexpr size_t MEM_SIZE = 812ull * 1024 * 1024;
        static constexpr size_t SCRATCH_SIZE = 544ull * 1024 * 1024;

        explicit BGEEmbeddingModel(const Config& config, const size_t mem_size = MEM_SIZE, const size_t scratch_size = SCRATCH_SIZE):
            BaseGenerationModel(BGE_M3_EMBEDDING, TextEmbedding, config, mem_size, scratch_size),
            w_ctx_({
                ggml_init({.mem_size = (GGML_TENSOR_SIZE + GGML_OBJECT_SIZE) * (5 + config.num_hidden_layers * 19), .mem_buffer = nullptr, .no_alloc = true}),
                config.dtype
            }),
            transformer_(&w_ctx_, config)
         {}

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

            GGML_ASSERT(ggml_used_mem(w_ctx_.g_ctx) == ggml_get_mem_size(w_ctx_.g_ctx));
        }

        XLMRoberta<BCEFinalNorm> & get_transformer() override {
            return transformer_;
        }

        size_t get_text_embedding_dim() override {
            return config_.hidden_size;
        }

    private:
        InitContext w_ctx_; // weight context
        XLMRoberta<BCEFinalNorm> transformer_;
    };



}


#endif //BGE_EMBEDDING_HPP
