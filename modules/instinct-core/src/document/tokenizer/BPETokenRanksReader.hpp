//
// Created by RobinQu on 2024/3/2.
//

#ifndef BPEFILEREADER_HPP
#define BPEFILEREADER_HPP

#include "CoreGlobals.hpp"
#include "Tokenizer.hpp"

namespace INSTINCT_CORE_NS {
    class BPETokenRanksReader {
    public:
        BPETokenRanksReader()=default;
        virtual ~BPETokenRanksReader()=default;
        BPETokenRanksReader(const BPETokenRanksReader&&)=delete;
        BPETokenRanksReader(BPETokenRanksReader&&)=delete;
        virtual BPETokenRanks Fetch()=0;
    };
}

#endif //BPEFILEREADER_HPP
