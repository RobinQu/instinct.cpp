//
// Created by RobinQu on 2024/6/8.
//

#ifndef FILECITATIONANNOTATIONCHAIN_HPP
#define FILECITATIONANNOTATIONCHAIN_HPP

#include "RetrievalGlobals.hpp"
#include "chain/MessageChain.hpp"


namespace INSTINCT_RETRIEVAL_NS {

    using CitationAnnotatingChain = MessageChain<CitationAnnotatingContext, AnswerWithCitations>;
    using CitationAnnotatingChainPtr = MessageChainPtr<CitationAnnotatingContext, AnswerWithCitations>;




}


#endif //FILECITATIONANNOTATIONCHAIN_HPP
