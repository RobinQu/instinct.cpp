//
// Created by RobinQu on 2024/3/2.
//

#ifndef BPEFILEREADER_HPP
#define BPEFILEREADER_HPP

#include "CoreGlobals.hpp"
#include "Tokenizer.hpp"

namespace INSTINCT_CORE_NS {
    class BPEFileReader {
    public:
        BPEFileReader()=default;
        virtual ~BPEFileReader()=default;
        BPEFileReader(const BPEFileReader&&)=delete;
        BPEFileReader(BPEFileReader&&)=delete;
        virtual BPETokenRanks Fetch()=0;
    };
}

#endif //BPEFILEREADER_HPP
