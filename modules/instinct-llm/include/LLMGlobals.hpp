//
// Created by RobinQu on 2024/2/13.
//

#ifndef MODELGLOBALS_H
#define MODELGLOBALS_H

#include "CoreGlobals.hpp"
#include <llm.pb.h>
#include <model/IEmbeddingModel.hpp>

#define INSTINCT_LLM_NS instinct::llm

namespace INSTINCT_LLM_NS {


    using TokenSize = unsigned long;
    using TokenId = unsigned long;


    static std::string DEFAULT_PROMPT_INPUT_KEY = "question";
    static std::string DEFAULT_ANSWER_OUTPUT_KEY = "answer";
    using MultiLineText = std::vector<std::string>;

    inline std::ostream& operator<<(std::ostream& ostrm, const Message& msg) {
        ostrm << "Message[role=" << msg.role() << ", content=" << msg.content() << "]";
        return ostrm;
    }

    inline std::ostream& operator<<(std::ostream& ostrm, const core::Embedding& embedding) {
        ostrm << "Embedding[";
        for (const auto& f: embedding) {
            ostrm << f << ", ";
        }
        return ostrm << "]";
    }



}


#endif //MODELGLOBALS_H
