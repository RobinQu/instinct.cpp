//
// Created by root on 5/25/24.
//

#ifndef CXX_TEST_MODEL_HPP
#define CXX_TEST_MODEL_HPP

#include <ggml.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <cstring>
#include <map>
#include <unistd.h>

#include "config.hpp"


namespace INSTINCT_TRANSFORMER_NS::models {

    struct GenerationConfig {
//    int max_length;
//    int max_context_length;
//    bool do_sample;
//    int top_k;
//    float top_p;
//    float temperature;
        int num_threads;
//    float presence_penalty;
//    float tfs_z;
//    std::string sampling;
    };

    ggml_tensor * ggml_init_tensor(struct ggml_tensor *tensor,
                                   enum   ggml_type      type,
                                   int                   n_dims,
                                   const int64_t       * ne)
    {
        struct ggml_tensor *result = tensor;

        *result = ggml_tensor {
                /*.type         =*/ type,
                /*.backend      =*/ GGML_BACKEND_TYPE_CPU,
                /*.buffer       =*/ NULL,
                /*.ne           =*/ { 1, 1, 1, 1 },
                /*.nb           =*/ { 0, 0, 0, 0 },
                /*.op           =*/ GGML_OP_NONE,
                /*.op_params    =*/ { 0 },
                /*.flags        =*/ 0,
                /*.grad         =*/ NULL,
                /*.src          =*/ { NULL },
                /*.perf_runs    =*/ 0,
                /*.perf_cycles  =*/ 0,
                /*.perf_time_us =*/ 0,
                /*.view_src     =*/ NULL,
                /*.view_offs    =*/ 0,
                /*.data         =*/ NULL,
                /*.name         =*/ { 0 },
                /*.extra        =*/ NULL,
                /*.padding      =*/ { 0 },
        };

        // TODO: this should not be needed as long as we don't rely on aligned SIMD loads
        //ggml_assert_aligned(result->data);

        for (int i = 0; i < n_dims; i++) {
            result->ne[i] = ne[i];
        }

        result->nb[0] = ggml_type_size(type);
        result->nb[1] = result->nb[0]*(result->ne[0]/ggml_blck_size(type));
        for (int i = 2; i < GGML_MAX_DIMS; i++) {
            result->nb[i] = result->nb[i - 1]*result->ne[i - 1];
        }

        return result;
    }



    class MappedFile
    {
    public:
        explicit MappedFile(const std::string &path) {
            int fd = open(path.c_str(), O_RDONLY);
            GGML_ASSERT(fd>0);

            struct stat sb {};
            GGML_ASSERT(fstat(fd, &sb) == 0);
            size = sb.st_size;

            data = (char *)mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
            GGML_ASSERT(data != MAP_FAILED);

            GGML_ASSERT(close(fd) == 0);
        }
        ~MappedFile() {
            GGML_ASSERT(munmap(data, size) == 0);
        }

    public:
        char *data;
        size_t size;
    };


    class ModelLoader {
    public:
        std::unique_ptr<MappedFile> mapped_file;
        const char *const data;
        size_t size;
        const char *ptr;

        int64_t offset_config;
        int64_t offset_tokenizer;
        int64_t offset_tensors;
        int model_type;
        int version;
        std::map<std::string, int64_t> tensor_dict;


        explicit ModelLoader(const std::string& model_file_path):
                mapped_file(std::make_unique<MappedFile>(model_file_path)),
                data(mapped_file->data),
                size(mapped_file->size),
                ptr(mapped_file->data),
                offset_config(0),
                offset_tokenizer(0),
                offset_tensors(0),
                model_type(-1),
                version(-1)
        {

        }

        [[nodiscard]] int64_t tell() const { return ptr - data; }

        void seek(int64_t offset, int whence) {
            if (whence == SEEK_SET)
                ptr = data + offset;
            else if (whence == SEEK_CUR)
                ptr += offset;
            else if (whence == SEEK_END)
                ptr = data + size + offset;
            else
                throw std::runtime_error("invalid seek mode");
        }

        template <typename T>
        T read_basic()
        {
            T obj = *(T *)ptr;
            ptr += sizeof(T);
            return obj;
        }

        std::string read_string(size_t length) {
            std::string s(ptr, ptr + length);
            ptr += length;
            return s;
        }

        void read_tensor(const std::string &name, ggml_tensor *tensor) {
            if (!tensor_dict.empty()) {
                auto search = tensor_dict.find(name);
                GGML_ASSERT(search != tensor_dict.end()); // "tensor not exist: " << name;
                seek(search->second, SEEK_SET);
            } else {
                // read and check tensor name
                int name_size = read_basic<int>();
                GGML_ASSERT(name_size == (int)name.size());
                std::string weight_name = read_string(name_size);
                GGML_ASSERT(weight_name == name);
            }

            ggml_set_name(tensor, name.c_str());

            // read and check tensor shape
            {
                int ndim = read_basic<int>();
                int n_dims = ggml_n_dims(tensor);

                // a quick fix
                if ((n_dims == 1) && (ndim == 2) && (tensor->ne[1] == 1))
                    n_dims = 2;

                GGML_ASSERT(ndim == n_dims);
                for (int i = ndim - 1; i >= 0; i--)
                {
                    int dim_size = read_basic<int>();
                    GGML_ASSERT(dim_size == tensor->ne[i]);
                }
            }

            // read and check tensor dtype
            {
                auto dtype = (ggml_type)read_basic<int>();
                GGML_ASSERT(dtype == tensor->type);
            }

            // map tensor data
            {
                constexpr int64_t MEM_ALIGNED = 16;
                const int64_t data_offset = (tell() + (MEM_ALIGNED - 1)) & ~(MEM_ALIGNED - 1);
                tensor->data = const_cast<char *>(data) + data_offset;
                seek(data_offset + ggml_nbytes(tensor), SEEK_SET);
            }
        }

        void load_all_tensors() {
            tensor_dict.clear(); //return;
            ggml_tensor t {};

            while (tell() < (int64_t)size)
            {
                std::string weight_name;

                // read and check tensor name
                {
                    int name_size = read_basic<int>();
                    weight_name = read_string(name_size);
                }

                tensor_dict.emplace(weight_name, tell());

                int64_t ne[4] = {1,1,1,1};

                // read and check tensor shape
                int ndim = read_basic<int>();
                for (int i = ndim - 1; i >= 0; i--)
                {
                    int dim_size = read_basic<int>();
                    ne[i] = dim_size;
                }

                // read and check tensor dtype
                auto dtype = (ggml_type)read_basic<int>();

                ggml_init_tensor(&t, dtype, ndim, ne);

                // map tensor data
                {
                    constexpr int64_t MEM_ALIGNED = 16;
                    const int64_t data_offset = (tell() + (MEM_ALIGNED - 1)) & ~(MEM_ALIGNED - 1);
                    seek(data_offset + ggml_nbytes(&t), SEEK_SET);
                }
            }
        }
    };

    enum ModelType {
        UNKNOWN = 0,
        BGE_M3_RERANKER = 0x10000103
    };

    enum ModelPurpose
    {
        Unknown = 0,
        TextEmbedding,
        Ranker,
    };

    class BaseModel {
    public:
        BaseModel(const ModelType model_type, const ModelPurpose model_purpose):
            model_type_(model_type),
            model_purpose_(model_purpose) {}

        virtual ~BaseModel()=default;
        virtual void load(ModelLoader& loader) = 0;
        virtual float qa_rank(const GenerationConfig& generation_config, const std::vector<int> &input_ids) = 0;
        virtual void text_embedding(const GenerationConfig& generation_config, const std::vector<int>& input_ids, std::vector<float>& output_embeding) = 0;
    protected:
        ModelType model_type_;
        ModelPurpose model_purpose_;
    };

    using ModelPtr = std::shared_ptr<BaseModel>;

    struct ForwardContext final {
        ggml_context *g_ctx = nullptr;
        ggml_cgraph *g_cgraph = nullptr;
        ggml_scratch g_scratch{};

        virtual ~ForwardContext() {
            ggml_free(g_ctx);
        }
    };

    struct InitContext {
        ggml_context *g_ctx;
        ggml_type dtype;
    };


    template<typename TransformerModel>
    class BaseGnerationModel: public BaseModel {
    public:

        BaseGnerationModel(
            const ModelType model_type, const ModelPurpose model_purpose,
            const BaseConfig& config, const size_t mem_size, const size_t scratch_size):
                BaseModel(model_type, model_purpose),
                GRAPH_SIZE(GGML_DEFAULT_GRAPH_SIZE),
                batch_input(true),
                logit_scale(-1.0f),
                config_(config),
                mem_size_(mem_size),
                mem_buffer_(new char[mem_size]),
                scratch_size_(scratch_size),
                scratch_buffer_(new char[scratch_size]),
                transformer_()
        {
            for (int i = 0; i < config.num_hidden_layers; i++)
                layer_ids.push_back(i);
        }

        float qa_rank(const GenerationConfig &config, const std::vector<int> &input_ids) override {
            const auto *lm = run_model(input_ids, config, 0);
            // "lm->type must be GGML_TYPE_F32"
            GGML_ASSERT(lm->type == GGML_TYPE_F32);
            // "ouput must be scaler"
            GGML_ASSERT((lm->ne[0] == 1) && (ggml_n_dims(lm) <= 1));

            return *(float *)lm->data;
        }

        void text_embedding(const GenerationConfig &generation_config, const std::vector<int> &input_ids,
            std::vector<float> &output_embeding) override {
            throw std::runtime_error("not implemented");
        }

    protected:

        virtual ggml_tensor *run_model(const std::vector<int> &input_ids,
                                       const GenerationConfig &gen_config,
                                       int past)
        {
            ForwardContext ctx;
            ctx.g_ctx = ggml_init({.mem_size = mem_size_, .mem_buffer = mem_buffer_.get(), .no_alloc = false});
            ctx.g_scratch = {.offs = 0, .size = scratch_size_, .data = scratch_buffer_.get()};

            int n_threads = input_ids.size() >= 32 && ggml_cpu_has_blas() && !ggml_cpu_has_gpublas() ? 1 : gen_config.num_threads;
            ctx.g_cgraph = ggml_new_graph_custom(ctx.g_ctx, GRAPH_SIZE, false);

            ggml_tensor *input_ids_tensor = ggml_new_tensor_1d(ctx.g_ctx, GGML_TYPE_I32, input_ids.size());
            std::memcpy(input_ids_tensor->data, input_ids.data(), ggml_nbytes(input_ids_tensor));

            ggml_tensor *r = transformer_.Forward(&ctx, input_ids_tensor, past);

            if (logit_scale > 0)
                r = ggml_scale_inplace(ctx.g_ctx, r, logit_scale);

            ggml_build_forward_expand(ctx.g_cgraph, r);
            ggml_graph_compute_with_ctx(ctx.g_ctx, ctx.g_cgraph, n_threads);

#ifdef GGML_PERF
            ggml_graph_print(ctx.g_cgraph);
#endif
            return r;
        }

        BaseConfig config_;
        size_t mem_size_;
        std::unique_ptr<char[]> mem_buffer_; // BLAS buffer
        size_t scratch_size_;
        std::unique_ptr<char[]> scratch_buffer_; // intermediate tensor buffer
    public:
        TransformerModel transformer_;
        size_t GRAPH_SIZE;
        bool batch_input;
        float logit_scale;
        std::vector<int> layer_ids;
    };


}


#endif //CXX_TEST_MODEL_HPP
