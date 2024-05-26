//
// Created by root on 5/25/24.
//

#ifndef CXX_TEST_LAYERS_HPP
#define CXX_TEST_LAYERS_HPP

#include <ggml.h>
#include <iostream>
#include <cmath>

namespace INSTINCT_TRANSFORMER_NS::layers {
    enum ActFunc {
        GELU,
        SILU,
        Tanh,
        RELU,
        RELU2
    };

    static ggml_tensor *inplace_act(ggml_context *ctx, ActFunc act, ggml_tensor *input) {
        switch (act) {
            case ActFunc::GELU:
                return ggml_gelu_inplace(ctx, input);
            case ActFunc::SILU:
                return ggml_silu_inplace(ctx, input);
            case ActFunc::Tanh:
                return ggml_tanh_inplace(ctx, input);
            case ActFunc::RELU:
                return ggml_relu_inplace(ctx, input);
            case ActFunc::RELU2: {
                ggml_tensor *output = ggml_relu_inplace(ctx, input);
                output = ggml_sqr_inplace(ctx, output);
                return output;
            }
            default:
                std::cerr << "not implemented act function: " << act;
                return nullptr;
        }
    }



    class Block {
    public:
        explicit Block(ggml_prec precision = GGML_PREC_DEFAULT, int id = 0) : precision_(precision), id_(id) {}

        virtual ~Block() = default;

        virtual ggml_tensor *Forward(ForwardContext *ctx, ggml_tensor *input) {
            throw std::runtime_error("Not implemented");
        }

        virtual ggml_tensor *Forward(ForwardContext *ctx, ggml_tensor *input, int n_past) {
            throw std::runtime_error("Not implemented");
        };

        virtual void set_id(int id) {
            id_ = id;
        }

    protected:
        ggml_prec precision_;
        int id_;
    };

    class Linear : public Block {
    public:
        explicit Linear(ggml_tensor *weight = nullptr, ggml_tensor *bias = nullptr) : weight(weight), bias(bias) {}

        Linear(InitContext *init_context, int in_features, int out_features, bool use_bias = true) :
                Linear(init_context, in_features, out_features, nullptr, use_bias) {}

        Linear(InitContext *init_context, int in_features, int out_features, ggml_tensor *weight, bool use_bias = true)
                :
                weight(weight ? weight : ggml_new_tensor_2d(init_context->g_ctx, init_context->dtype, in_features,
                                                            out_features)),
                bias(use_bias ? ggml_new_tensor_1d(init_context->g_ctx, GGML_TYPE_F32, out_features) : nullptr) {}

        [[nodiscard]] int in_features() const { return (int) weight->ne[0]; }

        [[nodiscard]] int out_features() const { return (int) weight->ne[1]; }

        ggml_tensor *Forward(ForwardContext *ctx, ggml_tensor *input) override {
            ggml_tensor *output = ggml_mul_mat(ctx->g_ctx, weight, input); // [seqlen, out_features]
            ggml_mul_mat_set_prec(output, precision_);
            if (bias) {
                output = ggml_add_inplace(ctx->g_ctx, output, bias);
            }
            return output;
        }

        ggml_tensor *weight;
        ggml_tensor *bias;

    };

    class LayerNorm : public Block {
    public:
        explicit LayerNorm(ggml_tensor *weight = nullptr, ggml_tensor *bias = nullptr) : weight(weight), bias(bias),
                                                                                         eps_(1e-5f) {}

        LayerNorm(InitContext *init_context, int normalized_shape, bool use_bias = true) :
                weight(ggml_new_tensor_1d(init_context->g_ctx, GGML_TYPE_F32, normalized_shape)),
                bias(use_bias ? ggml_new_tensor_1d(init_context->g_ctx, GGML_TYPE_F32, normalized_shape) : nullptr),
                eps_(1e-5) {}

        ggml_tensor *Forward(ForwardContext *ctx, ggml_tensor *input) override {
            // input: [seqlen, normalized_shape]
            ggml_tensor *output = ggml_norm_inplace(ctx->g_ctx, input, eps_);
            output = ggml_mul_inplace(ctx->g_ctx, output, weight);
            if (bias)
                output = ggml_add_inplace(ctx->g_ctx, output, bias);
            return output;
        }


        ggml_tensor *weight;
        ggml_tensor *bias;
    private:
        float eps_;
    };

    struct ShiftPending {
        int shift = 0;
        int total = 0;

        void clear() { shift = 0; }
    };

    class RobertaEmbedding : public Block {
    public:
        RobertaEmbedding() : word_weight(nullptr), position_weight(nullptr), indices(nullptr), pad_index(0) {}

        RobertaEmbedding(InitContext *init_context, int num_embeddings, int embedding_dim, int pos_max) :
                word_weight(
                        ggml_new_tensor_2d(init_context->g_ctx, init_context->dtype, embedding_dim, num_embeddings)),
                position_weight(ggml_new_tensor_2d(init_context->g_ctx, init_context->dtype, embedding_dim, pos_max)),
                indices(ggml_new_tensor_1d(init_context->g_ctx, GGML_TYPE_I32, pos_max)),
                ln(init_context, embedding_dim),
                pad_index(2) {
            indices->data = new char[ggml_nbytes(indices)];
            auto *p = (int32_t *) indices->data;
            for (int i = 0; i < pos_max; i++)
                p[i] = i;
        }

        ggml_tensor *Forward(ForwardContext *ctx, ggml_tensor *input, int n_past) override {
            int qlen = (int) input->ne[0];
            ggml_tensor *idx = ggml_view_1d(ctx->g_ctx, indices, qlen,
                                            (n_past + pad_index) * ggml_element_size(indices));

            ggml_tensor *output1 = ggml_get_rows(ctx->g_ctx, word_weight, input);
            ggml_tensor *output2 = ggml_get_rows(ctx->g_ctx, position_weight, idx);

            ggml_tensor *output = ggml_add_inplace(ctx->g_ctx, output1, output2);

            output = ln.Forward(ctx, output);
            return output;
        }

        ggml_tensor *word_weight;
        ggml_tensor *position_weight;
        ggml_tensor *indices;
        LayerNorm ln;
        int pad_index;
    };

    class CoreAttention : public Block {
    public:
        CoreAttention(InitContext *ctx, int num_attention_heads, int num_kv_heads, int max_length, ggml_type cache_type,
                      int k_cache_ele_num, int v_cache_ele_num)
                : num_attention_heads(num_attention_heads),
                  num_kv_heads(num_kv_heads),
                  k_cache(k_cache_ele_num > 0 ? ggml_new_tensor_1d(ctx->g_ctx, cache_type, k_cache_ele_num)
                                              : nullptr),
                  v_cache(v_cache_ele_num > 0 ? ggml_new_tensor_1d(ctx->g_ctx, cache_type, v_cache_ele_num)
                                              : nullptr),
                  pos(ggml_new_tensor_1d(ctx->g_ctx, GGML_TYPE_I32, max_length)),
                  max_length(max_length),
                  shift_pending_(),
                  attn_scaling_(true),
                  causal_(true) {
            if (k_cache_ele_num > 0) {
                k_cache->data = new char[ggml_nbytes(k_cache)]();
                ggml_set_name(k_cache, "k_cache");
            }
            if (v_cache_ele_num > 0) {
                v_cache->data = new char[ggml_nbytes(v_cache)]();
                ggml_set_name(v_cache, "v_cache");
            }
            pos->data = new char[ggml_nbytes(pos)]();
        }

        void shift_cache(int shift, int total) {
            shift_pending_ = ShiftPending(shift, total);
        }

        const int num_attention_heads;
        const int num_kv_heads;
        ggml_tensor *k_cache;
        ggml_tensor *v_cache;
        ggml_tensor *pos;
        const int max_length;

    protected:
        ShiftPending shift_pending_;
        bool attn_scaling_;
        bool causal_;

        virtual ggml_tensor *apply_pos_embedding_kq(ForwardContext *ctx, ggml_tensor *kq, int hidden_size, int qlen,
                                                    ggml_tensor *past) const {
            return kq;
        }

        // k: [heads, qlen, head_size]
        // q: [heads, qlen, head_size]
        // v: [heads, head_size, klen]
        virtual ggml_tensor *calc_attn_scores(
                ForwardContext *ctx,
                int hidden_size,
                const int n_past,
                const int qlen,
                ggml_tensor *key_layer,
                ggml_tensor *query_layer,
                ggml_tensor *value_layer) {
            const int head_size = hidden_size / num_attention_heads;

            // note auto-broadcasting in ggml_mul_mat for `repeat > 1`
            ggml_tensor *attn_scores = ggml_mul_mat(ctx->g_ctx, key_layer, query_layer); // [heads, qlen, klen]

            ggml_mul_mat_set_prec(attn_scores, precision_);

            if (attn_scaling_)
                attn_scores = ggml_scale_inplace(ctx->g_ctx, attn_scores, 1.f / sqrtf((float) head_size));

            attn_scores = apply_pos_embedding_kq(ctx, attn_scores, hidden_size, qlen, pos);

            // attn_masked = mask_past(attn_scores)
            struct ggml_tensor *attn_masked = causal_ ? ggml_diag_mask_inf_inplace(ctx->g_ctx, attn_scores, n_past)
                                                      : attn_scores;

            // attn_probs = soft_max(attn_masked)
            struct ggml_tensor *attn_probs = ggml_soft_max_inplace(ctx->g_ctx, attn_masked);

            ggml_tensor *context_layer = ggml_mul_mat(ctx->g_ctx, value_layer, attn_probs); // [heads, qlen, head_size]
            context_layer = ggml_reshape_2d(
                    ctx->g_ctx,
                    ggml_cont(ctx->g_ctx, ggml_permute(ctx->g_ctx, context_layer, 0, 2, 1, 3)),
                    hidden_size, qlen);

            return context_layer;
        }
    };

    static void fill_pos_vector(ggml_tensor *pos, int n_past, int qlen) {
        int *p = (int *) pos->data;
        for (int i = 0; i < qlen; i++)
            p[i] = n_past + i;
        pos->ne[0] = qlen;
    }


    class BaseAttention : public CoreAttention {
    public:
        BaseAttention(InitContext *ctx, int hidden_size, int num_attention_heads, int num_kv_heads, int head_dim,
                      int max_length,
                      bool qkv_bias, bool o_bias,
                      ggml_type cache_type, int cache_length)
                : CoreAttention(ctx, num_attention_heads, num_kv_heads, max_length, cache_type,
                                head_dim * num_kv_heads * cache_length,
                                head_dim * num_kv_heads * cache_length),
                  q_proj(ctx, hidden_size, head_dim * num_attention_heads, nullptr, qkv_bias),
                  k_proj(ctx, hidden_size, head_dim * num_kv_heads, nullptr, qkv_bias),
                  v_proj(ctx, hidden_size, head_dim * num_kv_heads, nullptr, qkv_bias),
                  o_proj(ctx, head_dim * num_attention_heads, hidden_size, nullptr, o_bias),
                  cache_length(cache_length) {
        }

        BaseAttention(InitContext *ctx, int hidden_size, int num_attention_heads, int num_kv_heads, int max_length,
                      bool qkv_bias, bool o_bias,
                      ggml_type cache_type, int cache_length)
                : BaseAttention(ctx, hidden_size, num_attention_heads, num_kv_heads, hidden_size / num_attention_heads,
                                max_length, qkv_bias, o_bias,
                                cache_type, cache_length) {}

        BaseAttention(InitContext *ctx, int hidden_size, int num_attention_heads, int num_kv_heads, int max_length,
                      bool qkv_bias, bool o_bias)
                : BaseAttention(ctx, hidden_size, num_attention_heads, num_kv_heads, hidden_size / num_attention_heads,
                                max_length, qkv_bias, o_bias,
                                GGML_TYPE_F16, max_length) {}

        BaseAttention(InitContext *ctx, int hidden_size, int num_attention_heads, int num_kv_heads, int head_dim,
                      int max_length,
                      bool qkv_bias, bool o_bias)
                : BaseAttention(ctx, hidden_size, num_attention_heads, num_kv_heads, head_dim, max_length, qkv_bias,
                                o_bias,
                                GGML_TYPE_F16, max_length) {}


    protected:
        // input & output: [qlen, heads, head_size]
        virtual ggml_tensor *apply_pos_embedding_k(ForwardContext *ctx, ggml_tensor *k, int hidden_size, int qlen,
                                                   ggml_tensor *past) const { return k; }

        virtual ggml_tensor *apply_pos_embedding_q(ForwardContext *ctx, ggml_tensor *q, int hidden_size, int qlen,
                                                   ggml_tensor *past) const { return q; }

//
        virtual void before_forward(ForwardContext *ctx, const int kv_hidden_size, const int n_past, const int qlen) {
            fill_pos_vector(pos, n_past, qlen);

            // shift cache
            if (shift_pending_.shift > 0) {
                int remain = shift_pending_.total - shift_pending_.shift;
                if (remain > 0) {
                    struct ggml_tensor *k_cache_remain = ggml_view_1d(ctx->g_ctx, k_cache, remain * kv_hidden_size,
                                                                      ggml_element_size(k_cache) * kv_hidden_size *
                                                                      shift_pending_.shift);
                    struct ggml_tensor *k_cache_1d = ggml_view_1d(ctx->g_ctx, k_cache, remain * kv_hidden_size,
                                                                  0);

                    struct ggml_tensor *v_cache_remain = ggml_view_2d(ctx->g_ctx, v_cache, remain, kv_hidden_size,
                                                                      cache_length * ggml_element_size(v_cache),
                                                                      shift_pending_.shift *
                                                                      ggml_element_size(v_cache));
                    struct ggml_tensor *v_cache_2d = ggml_view_2d(ctx->g_ctx, v_cache, remain, kv_hidden_size,
                                                                  cache_length * ggml_element_size(v_cache),
                                                                  0);

                    ggml_build_forward_expand(ctx->g_cgraph, ggml_cpy(ctx->g_ctx, k_cache_remain, k_cache_1d));
                    ggml_build_forward_expand(ctx->g_cgraph, ggml_cpy(ctx->g_ctx, v_cache_remain, v_cache_2d));
                }
                shift_pending_.clear();
            }
        }


        virtual void save_to_cache(ForwardContext *ctx, int kv_hidden_size, int n_past, int qlen, ggml_tensor *k,
                                   ggml_tensor *v) = 0;

        virtual ggml_tensor *get_k_from_cache(ForwardContext *ctx, int hidden_size, int n_past, int qlen) = 0;

        virtual ggml_tensor *get_v_from_cache(ForwardContext *ctx, int hidden_size, int n_past, int qlen) = 0;

        ggml_tensor *cross_attention(ForwardContext *ctx, const int hidden_size, const int n_past, const int qlen,
                                     ggml_tensor *q, ggml_tensor *k, ggml_tensor *v) {
            const int head_size = hidden_size / num_attention_heads;
            const int repeat = num_attention_heads / num_kv_heads;
            const int kv_hidden_size = hidden_size / repeat;

            // [qlen, heads, head_size]
            ggml_tensor *key_layer = ggml_reshape_3d(ctx->g_ctx, k, head_size, num_kv_heads, qlen);
            key_layer = apply_pos_embedding_k(ctx, key_layer, hidden_size, qlen, pos);

            // [qlen, heads, head_size]
            ggml_tensor *query_layer = ggml_reshape_3d(ctx->g_ctx, q, head_size, num_attention_heads, qlen);
            query_layer = apply_pos_embedding_q(ctx, query_layer, hidden_size, qlen, pos);

            if (!attn_scaling_)
                query_layer = ggml_scale(ctx->g_ctx, query_layer, 1.f / sqrtf((float) head_size));

            // store key and value to memory
            save_to_cache(ctx, kv_hidden_size, n_past, qlen, key_layer, v);

            query_layer = ggml_permute(ctx->g_ctx, query_layer, 0, 2, 1,
                                       3);                     // [heads, qlen, head_size]

            key_layer = get_k_from_cache(ctx, hidden_size, n_past, qlen);

            ggml_tensor *value_layer = get_v_from_cache(ctx, hidden_size, n_past, qlen);

            ggml_tensor *attn_scores = calc_attn_scores(ctx, hidden_size, n_past, qlen, key_layer, query_layer,
                                                        value_layer);
            return attn_scores;
        }

        int cache_length;

    public:
        Linear q_proj, k_proj, v_proj;
        Linear o_proj;
    };

    class BaseCachelessAttention : public BaseAttention {
    public:
        BaseCachelessAttention() = delete;

        BaseCachelessAttention(InitContext *ctx, int hidden_size, int num_attention_heads, int num_kv_heads,
                               int max_length, bool qkv_bias, bool o_bias)
                : BaseCachelessAttention(ctx, hidden_size, num_attention_heads, num_kv_heads,
                                         hidden_size / num_attention_heads, max_length, qkv_bias, o_bias) {}

        BaseCachelessAttention(InitContext *ctx, int hidden_size, int num_attention_heads, int num_kv_heads,
                               int head_dim, int max_length, bool qkv_bias, bool o_bias)
                : BaseAttention(ctx, hidden_size, num_attention_heads, num_kv_heads, head_dim, max_length, qkv_bias,
                                o_bias, GGML_TYPE_F16, 0),
                  raw_k(nullptr),
                  raw_v(nullptr) {}

    protected:
        void
        save_to_cache(ForwardContext *ctx, const int kv_hidden_size, const int n_past, const int qlen, ggml_tensor *k,
                      ggml_tensor *v) override {
            raw_k = k;
            raw_v = v;
        }

        // output: [heads, qlen, head_size]
        ggml_tensor *
        get_k_from_cache(ForwardContext *ctx, const int hidden_size, const int n_past, const int qlen) override {
            ggml_tensor *r = ggml_permute(ctx->g_ctx, raw_k, 0, 2, 1, 3);
            return r;
        }

        // output: [heads, head_size, klen]
        ggml_tensor *
        get_v_from_cache(ForwardContext *ctx, const int hidden_size, const int n_past, const int qlen) override {
            const int head_size = hidden_size / num_attention_heads;

            // [qlen, hidden_size] -> [heads, head_size, qlen]
            ggml_tensor *r = ggml_reshape_3d(ctx->g_ctx, raw_v, head_size, num_kv_heads,
                                             qlen);  // -> [qlen, heads, head_size]
            r = ggml_permute(ctx->g_ctx, r, 1, 2, 0, 3);   // [heads, head_size, qlen]
            r = ggml_cont(ctx->g_ctx, r);
            return r;
        }

    private:
        ggml_tensor *raw_k;
        ggml_tensor *raw_v;
    };


    enum RoPEMode {
        Interleaved = 0,        // IQIQ......IQ
        Original = 2,           // II...IQQ...Q
        GLM = 4,
    };


    template<class BaseAttn = BaseCachelessAttention>
    class BaseSelfAttention : public BaseAttn {
    public:
        BaseSelfAttention(InitContext *ctx, int hidden_size, int num_attention_heads, int max_length, bool qkv_bias,
                          bool o_bias)
                : BaseSelfAttention(ctx, hidden_size, num_attention_heads, num_attention_heads, max_length, qkv_bias,
                                    o_bias) {
        }

        BaseSelfAttention(InitContext *ctx, int hidden_size, int num_attention_heads, int num_kv_heads, int max_length,
                          bool qkv_bias, bool o_bias)
                : BaseSelfAttention(ctx, hidden_size, num_attention_heads, num_kv_heads,
                                    hidden_size / num_attention_heads, max_length, qkv_bias, o_bias) {
        }

        BaseSelfAttention(InitContext *ctx, int hidden_size, int num_attention_heads, int num_kv_heads, int head_dim,
                          int max_length, bool qkv_bias, bool o_bias)
                : BaseAttn(ctx, hidden_size, num_attention_heads, num_kv_heads, head_dim, max_length, qkv_bias, o_bias),
                  freq_base(10000.0f),
                  freq_scale(1.0f),
                  ext_factor(0.0f),
                  attn_factor(1.0f),
                  beta_fast(0.0f),
                  beta_slow(0.0f),
                  rope_dim(head_dim),
                  rope_mode(RoPEMode::Interleaved),
                  last_attn_scores(nullptr) {
        }

        using Block::Forward;

        ggml_tensor *Forward(ForwardContext *ctx, ggml_tensor *hidden_states, int n_past) override {
            const int hidden_size = BaseAttn::o_proj.in_features();
            const int qlen = (int) hidden_states->ne[1];
            const int repeat = BaseAttn::num_attention_heads / BaseAttn::num_kv_heads;
            const int kv_hidden_size = hidden_size / repeat;

            BaseAttn::before_forward(ctx, kv_hidden_size, n_past, qlen);

            ggml_tensor *tmpq = BaseAttn::q_proj.Forward(ctx, hidden_states);
            ggml_tensor *tmpk = BaseAttn::k_proj.Forward(ctx, hidden_states);
            ggml_tensor *tmpv = BaseAttn::v_proj.Forward(ctx, hidden_states);

            ggml_mul_mat_set_prec(tmpk, BaseAttn::precision_);
            ggml_mul_mat_set_prec(tmpq, BaseAttn::precision_);
            ggml_mul_mat_set_prec(tmpv, BaseAttn::precision_);

            last_attn_scores = BaseAttn::cross_attention(ctx, hidden_size, n_past, qlen, tmpq, tmpk, tmpv);

            ggml_tensor *attn_output = BaseAttn::o_proj.Forward(ctx, last_attn_scores);
            return attn_output;
        }

        ggml_tensor *get_last_attn_scores() {
            return last_attn_scores;
        }

    public:
        // rope param
        float freq_base;
        float freq_scale;
        float ext_factor;
        float attn_factor;
        float beta_fast;
        float beta_slow;
        int rope_dim;
        RoPEMode rope_mode;

    protected:
        // input & output: [qlen, heads, head_size]
        ggml_tensor *apply_pos_embedding_k(ForwardContext *ctx, ggml_tensor *k, int hidden_size, int qlen,
                                           ggml_tensor *past) const override {
            return ggml_rope_custom_inplace(ctx->g_ctx, k, past, rope_dim, rope_mode, 0, 0,
                                            freq_base, freq_scale, ext_factor, attn_factor, beta_fast,
                                            beta_slow);    // [qlen, heads, head_size]
        }

        ggml_tensor *apply_pos_embedding_q(ForwardContext *ctx, ggml_tensor *q, int hidden_size, int qlen,
                                           ggml_tensor *past) const override {
            return ggml_rope_custom_inplace(ctx->g_ctx, q, past, rope_dim, rope_mode, 0, 0,
                                            freq_base, freq_scale, ext_factor, attn_factor, beta_fast,
                                            beta_slow);    // [qlen, heads, head_size];
        }

    private:
        ggml_tensor *last_attn_scores;
    };
}

#endif //CXX_TEST_LAYERS_HPP
