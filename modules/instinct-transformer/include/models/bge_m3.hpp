//
// Created by root on 5/24/24.
//

#ifndef CXX_TEST_BGE_M3_HPP
#define CXX_TEST_BGE_M3_HPP
#include <ggml.h>
#include <vector>

#include "./config.hpp"
#include "./layers.hpp"
#include "./ops.hpp"
#include "./model.hpp"
#include "./tokenizer.hpp"

namespace INSTINCT_TRANSFORMER_NS::models::bge {
    using namespace INSTINCT_TRANSFORMER_NS::layers;
    using namespace INSTINCT_TRANSFORMER_NS::ops;
    using namespace INSTINCT_TRANSFORMER_NS::models;
    using namespace INSTINCT_TRANSFORMER_NS::tokenizer;

    struct Config: public BaseConfig {};

    class RobertaSelfAttention final: public BaseSelfAttention<BaseCachelessAttention> {
    public:
        RobertaSelfAttention(InitContext *ctx, int hidden_size, int num_attention_heads, int max_length)
                : RobertaSelfAttention(ctx, hidden_size, num_attention_heads, num_attention_heads, max_length) {}

        RobertaSelfAttention(InitContext *ctx, int hidden_size, int num_attention_heads, int num_kv_heads, int max_length)
                : RobertaSelfAttention(ctx, hidden_size, num_attention_heads, num_kv_heads, max_length, true, true) {}

        RobertaSelfAttention(InitContext *ctx, int hidden_size, int num_attention_heads, int num_kv_heads, int max_length, bool qkv_bias, bool o_bias)
                : BaseSelfAttention(ctx, hidden_size, num_attention_heads, num_kv_heads, max_length, qkv_bias, o_bias)
        {
            causal_ = false;
        }

    protected:
        // input & output: [qlen, heads, head_size]
        ggml_tensor *apply_pos_embedding_k(ForwardContext *ctx, ggml_tensor *k, int hidden_size, int qlen, ggml_tensor * past) const override
        {
            return k;
        }
        ggml_tensor *apply_pos_embedding_q(ForwardContext *ctx, ggml_tensor *q, int hidden_size, int qlen, ggml_tensor * past) const override
        {
            return q;
        }

    };


    class RobertaOutput final: public Block {
    public:

        RobertaOutput(InitContext *ctx, int hidden_size, bool use_bias = true): RobertaOutput(ctx, hidden_size, hidden_size, use_bias) {}

        RobertaOutput(InitContext *init_context, int hidden_size, int intermediate_size, bool use_bias = true):
                dense(init_context, intermediate_size, hidden_size, nullptr, use_bias),
                norm(init_context, hidden_size) {}

        ggml_tensor *Forward(ForwardContext *ctx, ggml_tensor *hidden_states, ggml_tensor *attention_output) {
            ggml_tensor *r = dense.forward(ctx, hidden_states);
            r = ggml_add_inplace(ctx->g_ctx, r, attention_output);
            r = norm.forward(ctx, r);
            return r;
        }

        Linear dense;
        LayerNorm norm;

    };


    class RobertaMLP final: public Block {
    public:
        RobertaMLP(InitContext* init_context, int hidden_size, int intermediate_size):
                RobertaMLP(init_context, hidden_size, intermediate_size, ActFunc::GELU, true) {}

        RobertaMLP(InitContext* init_context, int hidden_size, int intermediate_size, ActFunc act, bool bias):
                intermediate(init_context, hidden_size, intermediate_size, nullptr, bias),
                output(init_context, hidden_size, intermediate_size, bias),
                act(act) {}


        ggml_tensor *forward(ForwardContext *ctx, ggml_tensor *hidden_states) override {
            ggml_tensor *temp = intermediate.forward(ctx, hidden_states);
            temp = inplace_act(ctx->g_ctx, act, temp);
            temp = output.Forward(ctx, temp, hidden_states);
            return temp;
        }

        Linear intermediate;
        RobertaOutput output;
        ActFunc act;
    };


    class RobertaBlock final: public Block {
    public:
        RobertaBlock(InitContext *ctx, int hidden_size, int num_attention_heads, int intermediate_size, int num_kv_heads, int max_length)
                : RobertaBlock(ctx, hidden_size, num_attention_heads, intermediate_size, num_kv_heads, max_length, true, true)
        {}

        RobertaBlock(InitContext* init_context, int hidden_size, int num_attention_heads, int intermediate_size, int num_kv_heads, int max_length, bool qkv_bias, bool o_bias):
                attention(init_context, hidden_size, num_attention_heads, num_kv_heads, max_length, qkv_bias, o_bias),
                post_attention_layer_norm(init_context, hidden_size),
                mlp(init_context, hidden_size, intermediate_size),
                output_layer_norm(init_context, hidden_size)
        {}

        ggml_tensor *forward(ForwardContext *ctx, ggml_tensor *hidden_states, int n_past) override {
            ggml_tensor *attn_outputs = attention.forward(ctx, hidden_states, n_past);

            // see XLMRobertaSelfOutput
            ggml_tensor *sum = ggml_add(ctx->g_ctx, hidden_states, attn_outputs);
            ggml_tensor *attention_output = post_attention_layer_norm.forward(ctx, sum);

            ggml_tensor *r = mlp.forward(ctx, attention_output);
            return r;
        }


        RobertaSelfAttention attention;
        LayerNorm post_attention_layer_norm;
        RobertaMLP mlp;
        LayerNorm output_layer_norm;

    };


    class RobertaClassificationHead final: public Block {
    public:
        RobertaClassificationHead()=default;
        RobertaClassificationHead(InitContext *init_context, int hidden_size):
                dense(init_context, hidden_size, hidden_size, nullptr),
                activation(ActFunc::Tanh),
                out_proj(init_context, hidden_size, 1, nullptr)
        {}


        ggml_tensor *forward(ForwardContext *ctx, ggml_tensor *hidden_states) override {
            int hidden_size = (int)hidden_states->ne[0];

            // We "pool" the model by simply taking the hidden state corresponding to the first token.
            ggml_tensor *first_token_tensor = ggml_view_2d(ctx->g_ctx, hidden_states, hidden_size, 1,
                                                           hidden_size * ggml_element_size(hidden_states), 0);
            ggml_tensor *output = dense.forward(ctx, first_token_tensor);
            output = inplace_act(ctx->g_ctx, activation, output);
            output = out_proj.forward(ctx, output);
            output = ggml_map_custom1(ctx->g_ctx, output, ggml_compute_forward_sigmoid, 1, nullptr);
            return output;
        }

        Linear dense;
        ActFunc activation = ActFunc::Tanh;
        Linear out_proj;
    };


    class XLMRoberta final: public Block {
        BaseConfig config_;
    public:
        XLMRoberta(InitContext* init_context, const BaseConfig& config):
                config_(config),
                word_embeddings(init_context, config.vocab_size, config.hidden_size, config.max_length),
                layers(),
                final(init_context, config.hidden_size)
        {
            layers.reserve(config.num_hidden_layers);
            for(int layer_id=0; layer_id<config_.num_hidden_layers; ++layer_id) {
                layers.emplace_back(
                        init_context,
                        config.hidden_size,
                        config.num_attention_heads,
                        config.intermediate_size,
                        config.num_attention_heads,
                        config.max_length
                );
                layers[layer_id].set_id(layer_id);
            }
        }

        ggml_tensor *forward(ForwardContext *ctx, ggml_tensor *input_ids, int n_past) override {
            ggml_tensor *hidden_states = word_embeddings.forward(ctx, input_ids, n_past);
            for (auto &layer : layers) {
                ggml_set_scratch(ctx->g_ctx, ctx->g_scratch);
                hidden_states = layer.forward(ctx, hidden_states, n_past);
            }
            return final_steps(ctx, input_ids, hidden_states);
        }

        RobertaEmbedding word_embeddings;
        std::vector<RobertaBlock> layers;
        RobertaClassificationHead final;

    private:
        ggml_tensor *final_steps(ForwardContext *ctx, ggml_tensor *input_ids, ggml_tensor *hidden_states)
        {
            ggml_set_scratch(ctx->g_ctx, {.offs = 0, .size = 0, .data = nullptr});
            ggml_tensor *transformer_outputs = final.forward(ctx, hidden_states);
            return transformer_outputs;
        }

    };


    class BGEM3RerankerModel final: public BaseGnerationModel<XLMRoberta> {
    public:
        static constexpr size_t MEM_SIZE = 812ull * 1024 * 1024;
        static constexpr size_t SCRATCH_SIZE = 544ull * 1024 * 1024;

        explicit BGEM3RerankerModel(const Config& config, const size_t mem_size = MEM_SIZE, const size_t scratch_size = SCRATCH_SIZE):
            BaseGnerationModel(BGE_M3_RERANKER, Ranker, config, mem_size, scratch_size),
                w_ctx_(
                       {
                           ggml_init({.mem_size = ((9 + config.num_hidden_layers * 19) * (GGML_TENSOR_SIZE + GGML_OBJECT_SIZE)), .mem_buffer = nullptr, .no_alloc = true}),
                           config.dtype
                       }
                ),
                transformer_(&w_ctx_, config_)
        {
        }

        XLMRoberta& get_transformer() override {
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
        XLMRoberta transformer_;

    };


    class Tokenizer final: public BaseTokenizer {
    public:
        explicit Tokenizer(const BaseConfig &config) : BaseTokenizer(config) {}

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
}





#endif //CXX_TEST_BGE_M3_HPP
