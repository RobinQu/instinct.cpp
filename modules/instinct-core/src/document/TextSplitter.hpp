//
// Created by RobinQu on 2024/2/27.
//

#ifndef TEXTSPLITTER_HPP
#define TEXTSPLITTER_HPP

#include "Document.hpp"
#include "CoreGlobals.hpp"


namespace INSTINCT_CORE_NS {

    class TextSplitter {
    public:
        virtual ~TextSplitter() =default;
        virtual std::vector<std::string> SplitText(const std::string& text) = 0;
    };

}

#endif //TEXTSPLITTER_HPP
