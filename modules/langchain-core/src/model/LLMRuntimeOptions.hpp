//
// Created by RobinQu on 2024/2/22.
//

#ifndef LANGUAGEMODELRUNTIMEOPTIONS_HPP
#define LANGUAGEMODELRUNTIMEOPTIONS_HPP


#include "CoreGlobals.hpp"

LC_CORE_NS {
    struct LLMRuntimeOptions {
        static auto Defaults() {
            return LLMRuntimeOptions{};
        }

        /**
         * \brief custom stop words
         */
        const std::vector<std::string> stop_words;
    };
}

#endif //LANGUAGEMODELRUNTIMEOPTIONS_HPP
