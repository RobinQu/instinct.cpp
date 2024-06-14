//
// Created by root on 5/24/24.
//

#ifndef CXX_TEST_CONFIG_HPP
#define CXX_TEST_CONFIG_HPP
#include <ggml.h>
#include "./globals.hpp"


namespace INSTINCT_TRANSFORMER_NS {
    struct BaseConfig
    {
        // common attributes
        ggml_type dtype;
        int vocab_size;
        int hidden_size;
        int num_attention_heads;
        int num_hidden_layers;
        int intermediate_size;
        // for sequence generation
        int max_length;
        // for tokenizer
        int bos_token_id;
        int eos_token_id;
        int pad_token_id;
        int sep_token_id;
    };
}



#endif //CXX_TEST_CONFIG_HPP
