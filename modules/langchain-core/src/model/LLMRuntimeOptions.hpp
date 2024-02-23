//
// Created by RobinQu on 2024/2/22.
//

#ifndef LANGUAGEMODELRUNTIMEOPTIONS_HPP
#define LANGUAGEMODELRUNTIMEOPTIONS_HPP


#include "CoreGlobals.hpp"

LC_CORE_NS {
    struct LLMRuntimeOptions {
        static auto Defaults() {
            return LLMRuntimeOptions{


            };
        }

        /**
         * \brief custom stop words
         */
        std::vector<std::string> stop_words;

        std::string model_name = "llama2";
    };
}

#endif //LANGUAGEMODELRUNTIMEOPTIONS_HPP
