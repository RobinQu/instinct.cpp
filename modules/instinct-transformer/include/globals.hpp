//
// Created by RobinQu on 2024/5/26.
//

#ifndef GLOBALS_HPP
#define GLOBALS_HPP
#include <ggml.h>

#define INSTINCT_TRANSFORMER_NS instinct::transformer


namespace INSTINCT_TRANSFORMER_NS {

    static const char * print_system_info() {
        static std::string s;

        s  = "";
        s += "AVX = "         + std::to_string(ggml_cpu_has_avx())         + " | ";
        s += "AVX_VNNI = "    + std::to_string(ggml_cpu_has_avx_vnni())    + " | ";
        s += "AVX2 = "        + std::to_string(ggml_cpu_has_avx2())        + " | ";
        s += "AVX512 = "      + std::to_string(ggml_cpu_has_avx512())      + " | ";
        s += "AVX512_VBMI = " + std::to_string(ggml_cpu_has_avx512_vbmi()) + " | ";
        s += "AVX512_VNNI = " + std::to_string(ggml_cpu_has_avx512_vnni()) + " | ";
        // s += "AVX512_BF16 = " + std::to_string(ggml_cpu_has_avx512_bf16()) + " | ";
        s += "FMA = "         + std::to_string(ggml_cpu_has_fma())         + " | ";
        s += "NEON = "        + std::to_string(ggml_cpu_has_neon())        + " | ";
        // s += "SVE = "         + std::to_string(ggml_cpu_has_sve())         + " | ";
        s += "ARM_FMA = "     + std::to_string(ggml_cpu_has_arm_fma())     + " | ";
        s += "F16C = "        + std::to_string(ggml_cpu_has_f16c())        + " | ";
        s += "FP16_VA = "     + std::to_string(ggml_cpu_has_fp16_va())     + " | ";
        s += "WASM_SIMD = "   + std::to_string(ggml_cpu_has_wasm_simd())   + " | ";
        s += "BLAS = "        + std::to_string(ggml_cpu_has_blas())        + " | ";
        s += "SSE3 = "        + std::to_string(ggml_cpu_has_sse3())        + " | ";
        s += "SSSE3 = "       + std::to_string(ggml_cpu_has_ssse3())       + " | ";
        s += "VSX = "         + std::to_string(ggml_cpu_has_vsx())         + " | ";
        s += "MATMUL_INT8 = " + std::to_string(ggml_cpu_has_matmul_int8()) + " | ";
#ifdef GGML_USE_LLAMAFILE
        s += "LLAMAFILE = 1 | ";
#else
        s += "LLAMAFILE = 0 | ";
#endif

        return s.c_str();
    }
}

#endif //GLOBALS_HPP
