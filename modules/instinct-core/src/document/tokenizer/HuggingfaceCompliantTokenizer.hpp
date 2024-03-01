//
// Created by RobinQu on 2024/2/29.
//

#ifndef HUGGINGFACECOMPLIANTTOKENIZER_HPP
#define HUGGINGFACECOMPLIANTTOKENIZER_HPP

#include "CoreGlobals.hpp"
#include "PretrainedTokenizer.hpp"

namespace INSTINCT_CORE_NS {
    class HuggingFaceCompliantTokenzier: public PretrainedTokenizer {
    public:
        std::vector<int32_t> Encode(const std::string& text) override;

        std::string Decode(const std::vector<int32_t>& ids) override;

        // size_t GetVocabSize() override;
        //
        // std::string IdToToken(int32_t id) override;
        //
        // int32_t TokenToId(const std::string& token) override;
    };
}


#endif //HUGGINGFACECOMPLIANTTOKENIZER_HPP
